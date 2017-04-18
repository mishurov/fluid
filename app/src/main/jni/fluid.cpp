#include <stdlib.h>
#include <math.h> 
#include "fluid.h"

#include <android/log.h>

using namespace std;

//int iterations = 32;
int iterations = 16;
int mouse_force = 1;
// not used float resolution = 0.5;
float cursor_size = 50;
float step = 1.0 / 60.0;

//vector<float> fg_color = { 0.16, 0.01, 0.36 };
vector<float> fg_color = { 0.0, 0.0, 0.0 };
vector<float> bg_color = { 1.0, 1.0, 1.0 };


vector<float> ArgbHexStringToRrbVecFloat(string str) {
	string r_str = str.substr(2,2);
	string g_str = str.substr(4,2);
	string b_str = str.substr(6,2);
	int r_int = (int)strtol(r_str.c_str(), NULL, 16);
	int g_int = (int)strtol(g_str.c_str(), NULL, 16);
	int b_int = (int)strtol(b_str.c_str(), NULL, 16);
	float r = (float) r_int / (float) 255.0;
	float g = (float) g_int / (float) 255.0;
	float b = (float) b_int / (float) 255.0;
	return { r, g, b };
}

void FluidSetPrefs(
	string fg_color_str, string bg_color_str,
	int iterations_arg, int cursor_size_arg) {
	fg_color = ArgbHexStringToRrbVecFloat(fg_color_str);
	bg_color = ArgbHexStringToRrbVecFloat(bg_color_str);
	iterations = iterations_arg;
	cursor_size = (float) (cursor_size_arg * 10);
}

ComputeKernel advect_velocity;
ComputeKernel advect_temperature;
ComputeKernel advect_density;
ComputeKernel apply_buoyancy;
ComputeKernel apply_impulse_temperature;
ComputeKernel apply_impulse_density;
ComputeKernel apply_divergence;
ComputeKernel compute_jacobi;
ComputeKernel subtract_gradient;
ComputeKernel draw;

FBO pressure_ping;
FBO pressure_pong;
FBO velocity_ping;
FBO velocity_pong;
FBO density_ping;
FBO density_pong;
FBO temperature_ping;
FBO temperature_pong;
FBO divergence_pong;

float px_x;
float px_y;


bool HasFloatLuminanceFBOSupport() {
	FBO fbo(32, 32, GL_FLOAT, GL_LUMINANCE);
	return fbo.supported();
}

void FluidInit(int width, int height) {
	glViewport(0, 0, width, height);
	glLineWidth(1.0);
	GLenum format = HasFloatLuminanceFBOSupport() ? GL_LUMINANCE : GL_RGBA;
	px_x = 1.0 / (float) width;
	px_y = 1.0 / (float) height;
	vector<float> px = { px_x, px_y };
	vector<float> px1 = { 1.0, (float) width / (float) height };
	
	Mesh inside(
		GL_TRIANGLES,
		ScreenQuad(1.0 - px_x * 2.0, 1.0 - px_y * 2.0),
		{ 0, 1, 2, 3, 4, 5 },
		{
			{"position",
				{
					{"size", 3},
					{"stride", 0},
					{"offset", 0},
				},
			},
		}
	);

	Mesh all(
		GL_TRIANGLES,
		ScreenQuad(1.0, 1.0),
		{ 0, 1, 2, 3, 4, 5 },
		{
			{"position",
				{
					{"size", 3},
					{"stride", 0},
					{"offset", 0},
				},
			},
		}
	);

	Mesh boundary(
		GL_LINES,
		{
			-1+px_x*0, -1+px_y*0,
			-1+px_x*0, -1+px_y*2,

			 1-px_x*0, -1+px_y*0,
			 1-px_x*0, -1+px_y*2,

			// top
			-1+px_x*0,  1-px_y*0,
			-1+px_x*0,  1-px_y*2,

			 1-px_x*0,  1-px_y*0,
			 1-px_x*0,  1-px_y*2,

			// left
			-1+px_x*0,  1-px_y*0,
			-1+px_x*2,  1-px_y*0,

			-1+px_x*0, -1+px_y*0,
			-1+px_x*2, -1+px_y*0,

			// right
			 1-px_x*0,  1-px_y*0,
			 1-px_x*2,  1-px_y*0,

			 1-px_x*0, -1+px_y*0,
			 1-px_x*2, -1+px_y*0
		},
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{
			{"position",
				{
					{"size", 2},
					{"stride", 4},
					{"offset", 0},
				},
			},
		 	{"offset", 
				{
					{"size", 2},
					{"stride", 4},
					{"offset", 8},
				},
			},
		}
	);
	
	/* inkling */
	Shader advect("shaders/kernel.glsl", "shaders/advect.glsl");
	Shader jacobi("shaders/kernel.glsl", "shaders/jacobi.glsl");
	Shader gradient("shaders/kernel.glsl", "shaders/gradient.glsl");
	Shader divergence("shaders/kernel.glsl", "shaders/divergence.glsl");
	Shader impulse("shaders/kernel.glsl", "shaders/splat.glsl");
	Shader buoyancy("shaders/kernel.glsl", "shaders/buoyancy.glsl");
	Shader visualize("shaders/kernel.glsl", "shaders/visualize.glsl");
	Shader add_field("shaders/cursor.glsl", "shaders/add_field.glsl");

	velocity_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	velocity_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	density_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	density_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	pressure_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	pressure_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	temperature_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	temperature_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	divergence_pong = FBO(width, height, GL_FLOAT, GL_RGBA);

	advect_velocity = ComputeKernel(
		advect,
		inside,
		{
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"dissipation", {FBO(), {0.999}}},
			{"velocity", {velocity_ping, vector<float>()}},
			{"source", {velocity_ping, vector<float>()}},
			{"dt", {FBO(), {step}}},
		},
		velocity_pong,
		"",
		false,
		false
	);

	advect_temperature = ComputeKernel(
		advect,
		inside,
		{
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"dissipation", {FBO(), {0.95}}},
			{"velocity", {velocity_ping, vector<float>()}},
			{"source", {temperature_ping, vector<float>()}},
			{"dt", {FBO(), {step}}},
		},
		temperature_pong,
		"",
		false,
		false
	);

	advect_density = ComputeKernel(
		advect,
		inside,
		{
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"dissipation", {FBO(), {1.0}}},
			{"velocity", {velocity_ping, vector<float>()}},
			{"source", {density_ping, vector<float>()}},
			{"dt", {FBO(), {step}}},
		},
		density_pong,
		"",
		false,
		false
	);

	apply_buoyancy = ComputeKernel(
		buoyancy,
		all,
		{
			{"ambient_temperature", {FBO(), {0.0}}},
			{"sigma", {FBO(), {1.5}}}, // Smoke Buoyancy
			{"kappa", {FBO(), {0.1}}}, // Smoke Weight
			{"velocity", {velocity_ping, vector<float>()}},
			{"temperature", {temperature_ping, vector<float>()}},
			{"density", {density_ping, vector<float>()}},
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"dt", {FBO(), {step}}},
		},
		velocity_pong,
		"",
		false,
		false
	);

	Mesh cursor(
		GL_TRIANGLES,
		ScreenQuad(px_x * cursor_size * 2, px_y * cursor_size *2 ),
		{ 0, 1, 2, 3, 4, 5 },
		{
			{"position",
				{
					{"size", 3},
					{"stride", 0},
					{"offset", 0},
				},
			},
		}
	);

	apply_impulse_density = ComputeKernel(
		add_field,
		cursor,
		{
			{"px", {FBO(), px}},
			{"force", {FBO(), {0.5, 0.2}}},
			{"center", {FBO(), {0.1, 0.4}}},
			{"scale", {FBO(), {cursor_size * px_x, cursor_size * px_y}}},
		},
		temperature_ping,
		"add",
		false,
		false
	);

	apply_impulse_temperature = ComputeKernel(
		add_field,
		cursor,
		{
			{"px", {FBO(), px}},
			{"force", {FBO(), {0.5, 0.2}}},
			{"center", {FBO(), {0.1, 0.4}}},
			{"scale", {FBO(), {cursor_size * px_x, cursor_size * px_y}}},
		},
		density_ping,
		"add",
		false,
		false
	);

	apply_divergence = ComputeKernel(
		divergence,
		all,
		{
			{"velocity", {velocity_ping, vector<float>()}},
			{"px", {FBO(), px}},
		},
		divergence_pong,
		"",
		false,
		false
	);

	compute_jacobi = ComputeKernel(
		jacobi,
		all,
		{
			{"pressure", {pressure_ping, vector<float>()}},
			{"divergence", {divergence_pong, vector<float>()}},
			{"alpha", {FBO(), {-1.0}}},
			{"beta", {FBO(), {0.25}}},
			{"px", {FBO(), px}},
		},
		pressure_pong,
		"",
		false,
		false
	);

	subtract_gradient = ComputeKernel(
		gradient,
		all,
		{
			{"scale", {FBO(), {1.0}}},
			{"pressure", {pressure_ping, vector<float>()}},
			{"velocity", {velocity_ping, vector<float>()}},
			{"px", {FBO(), px}},
		},
		velocity_pong,
		"",
		false,
		false
	);

	draw = ComputeKernel(
		visualize,
		all,
		{
			{"sampler", {pressure_pong, vector<float>()}},
			{"bg_color", {FBO(), bg_color}},
			{"fg_color", {FBO(), fg_color}},
			{"px", {FBO(), px}},
		},
		FBO(),
		"",
		false,
		false
	);
}

float x_0 = 0;
float y_0 = 0;
float x_1 = 0;
float y_1 = 0;
bool is_cursor_down = false;

void FluidTouch(bool is_down, float x, float y) {
	is_cursor_down = is_down;
	x_1 = x;
	y_1 = y;
}

float pi = 3.14159265;
vector<float> gravity = {0, 0};

void FluidRotate(int angle) {
	angle += 90;
	float theta = (float) angle * pi / 180.0;
	gravity = {(float) cos(theta), (float) sin(theta)};
}

void SwapBuffers(FBO *fbo0, FBO *fbo1) {
	FBO swap = *fbo0;
	*fbo0 = *fbo1;
	*fbo1 = swap;
}

void FluidUpdate(float elapsed_time) {
	UniformsMap uniforms = {
		{"source", {velocity_ping, vector<float>()}},
		{"velocity", {velocity_ping, vector<float>()}}
	};
    advect_velocity.SetUniforms(uniforms);
    advect_velocity.SetFBO(velocity_pong);
	advect_velocity.Run();
	SwapBuffers(&velocity_ping, &velocity_pong);

	uniforms = {
		{"source", {temperature_ping, vector<float>()}},
		{"velocity", {velocity_ping, vector<float>()}}
	};
    advect_temperature.SetUniforms(uniforms);
    advect_temperature.SetFBO(temperature_pong);
	advect_temperature.Run();
	SwapBuffers(&temperature_ping, &temperature_pong);

	uniforms = {
		{"source", {density_ping, vector<float>()}},
		{"velocity", {velocity_ping, vector<float>()}}
	};
    advect_density.SetUniforms(uniforms);
    advect_density.SetFBO(density_pong);
	advect_density.Run();
	SwapBuffers(&density_ping, &density_pong);
	
	uniforms = {
		{"velocity", {velocity_ping, vector<float>()}},
		{"temperature", {temperature_ping, vector<float>()}},
		{"density", {density_ping, vector<float>()}},
		{"gravity", {FBO(), gravity}},
	};
    apply_buoyancy.SetUniforms(uniforms);
    apply_buoyancy.SetFBO(velocity_pong);
	apply_buoyancy.Run();
	SwapBuffers(&velocity_ping, &velocity_pong);


	float xd = x_1 - x_0;
	float yd = y_1 - y_0;
	x_0 = x_1;
	y_0 = y_1;

	UniformsMap force = {
		{"force", {FBO(),
			{
				xd * px_x * cursor_size * mouse_force,
				-yd * px_y * cursor_size * mouse_force
			}
		}},
		{"center", {FBO(),
			{
				x_0 * px_x * 2 - 1,
				(y_0 * px_y * 2 - 1) * -1
			}
		}}
	};
	
	float min_d = 5.0;

	float force_avg = (float) fabs(xd) + (float) fabs(yd);
	if (is_cursor_down) {
		force_avg = max(min_d, force_avg);
	} else {
		force_avg = 0;
	}
	// temperature
	float size_factor = cursor_size * 0.003;
	UniformsMap module_force = {
		{"force", {FBO(),
			{
				force_avg * px_x * mouse_force / size_factor,
				force_avg * px_y * mouse_force / size_factor,
			}
		}},
	};

	apply_impulse_density.SetUniforms(force);
	apply_impulse_density.SetUniforms(module_force);
    apply_impulse_density.SetFBO(density_ping);
	apply_impulse_density.Run();

	apply_impulse_temperature.SetUniforms(force);
	apply_impulse_temperature.SetUniforms(module_force);
    apply_impulse_temperature.SetFBO(temperature_ping);
	apply_impulse_temperature.Run();

	apply_impulse_temperature.SetUniforms(force);
    apply_impulse_temperature.SetFBO(velocity_ping);
	apply_impulse_temperature.Run();


	uniforms = {
		{"velocity", {velocity_ping, vector<float>()}},
	};
    apply_divergence.SetUniforms(uniforms);
    apply_divergence.SetFBO(divergence_pong);
	apply_divergence.Run();
	
	//pressure_ping.Clear(0, 0);
	for(int i = 0; i < iterations; i++) {
		uniforms = {
			{"divergence", {divergence_pong, vector<float>()}},
			{"pressure", {pressure_ping, vector<float>()}},
		};
    	compute_jacobi.SetUniforms(uniforms);
    	compute_jacobi.SetFBO(pressure_pong);
		compute_jacobi.Run();
		SwapBuffers(&pressure_ping, &pressure_pong);
	}
	uniforms = {
		{"velocity", {velocity_ping, vector<float>()}},
		{"pressure", {pressure_ping, vector<float>()}},
	};
    subtract_gradient.SetUniforms(uniforms);
    subtract_gradient.SetFBO(velocity_pong);
	subtract_gradient.Run();
	SwapBuffers(&velocity_ping, &velocity_pong);

	uniforms = {
		{"bg_color", {FBO(), bg_color}},
		{"fg_color", {FBO(), fg_color}},
		{"sampler", {density_ping, vector<float>()}},
	};
    draw.SetUniforms(uniforms);
	draw.Run();

}

