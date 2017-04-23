#include <stdlib.h>
#include <math.h> 
#include "fluid.h"

#include <android/log.h>

using namespace std;

static int iterations = 16;
static int mouse_force = 1;
static float cursor_size = 50;
static float step = 1.0 / 60.0;
static vector<float> fg_color = { 0.0, 0.0, 0.0 };
static vector<float> bg_color = { 1.0, 1.0, 1.0 };


vector<float> HStringToFloat3(string str) {
	vector<float> ret = vector<float>();
	for (int pos = 2; pos <= 6; pos += 2 ) {
		string color_str = str.substr(pos, 2);
		int color_int = (int) strtol(color_str.c_str(), NULL, 16);
		float color = ((float) color_int) / (float) 255.0;
		ret.push_back(color);
	}
	return ret;
}


void FluidSetPrefs(
	string fg_color_str, string bg_color_str,
	int iterations_arg, int cursor_size_arg) {
	fg_color = HStringToFloat3(fg_color_str);
	bg_color = HStringToFloat3(bg_color_str);
	iterations = iterations_arg;
	cursor_size = (float) (cursor_size_arg * 10);
}

static ComputeKernel fill_zeroes;
static ComputeKernel advect;
static ComputeKernel impulse;
static ComputeKernel apply_buoyancy;
static ComputeKernel apply_divergence;
static ComputeKernel compute_jacobi;
static ComputeKernel subtract_gradient;
static ComputeKernel draw;

static FBO pressure_ping;
static FBO pressure_pong;
static FBO velocity_ping;
static FBO velocity_pong;
static FBO density_ping;
static FBO density_pong;
static FBO temperature_ping;
static FBO temperature_pong;
static FBO divergence_ping;

static Shader advect_field;
static Shader add_field;
static Shader fill_packed_zeroes;
static Shader jacobi;
static Shader gradient;
static Shader divergence;
static Shader buoyancy;
static Shader visualize;

static Mesh inside;
static Mesh all;
static Mesh boundary;

static float px_x;
static float px_y;
static vector<float> px;
static vector<float> px1;
static float width;
static float height;

bool busy = false;

void ClearFBO(FBO *fbo, float size) {
	UniformsMap uniforms = {
		{"vector_size", {FBO(), {size}}},
	};
    fill_zeroes.SetUniforms(uniforms);
    fill_zeroes.SetFBO(*fbo);
   	fill_zeroes.Run();
}

void FluidSurface(int width_arg, int height_arg) {
	busy = true;
	width = width_arg;
	height = height_arg;
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	px_x = 1.0 / (float) width;
	px_y = 1.0 / (float) height;
	px = { px_x, px_y };
	px1 = { 1.0, (float) width / (float) height };
	
	inside = Mesh(
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

	all = Mesh(
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

	boundary = Mesh(
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

	FluidInit();

	ClearFBO(&density_ping, 1.0);
	ClearFBO(&density_pong, 1.0);
	ClearFBO(&velocity_ping, 2.0);
	ClearFBO(&velocity_pong, 2.0);
	ClearFBO(&temperature_ping, 1.0);
	ClearFBO(&temperature_pong, 1.0);

	busy = false;
}


void FluidInit() {
	velocity_ping = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	velocity_pong = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	density_ping = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	density_pong = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	pressure_ping = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	pressure_pong = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	temperature_ping = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	temperature_pong = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	divergence_ping = FBO(width, height, GL_UNSIGNED_BYTE, GL_RGBA);
	
	advect_field = Shader(
		"shaders/surface.glsl", "shaders/advect_field.glsl"
	);
	add_field = Shader("shaders/surface.glsl", "shaders/add_field.glsl");
	fill_packed_zeroes = Shader(
		"shaders/surface.glsl", "shaders/fill_packed_zeroes.glsl"
	);
	jacobi = Shader("shaders/surface.glsl", "shaders/jacobi.glsl");
	gradient = Shader("shaders/surface.glsl", "shaders/gradient.glsl");
	divergence = Shader("shaders/surface.glsl", "shaders/divergence.glsl");
	buoyancy = Shader("shaders/surface.glsl", "shaders/buoyancy.glsl");
	visualize = Shader("shaders/surface.glsl", "shaders/visualize.glsl");

	fill_zeroes = ComputeKernel(
		fill_packed_zeroes,
		all,
		{
			{"vector_size", {FBO(), {1.0}}},
		},
		density_ping,
		"", false, false
	);

	advect = ComputeKernel(
		advect_field,
		inside,
		{
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"dissipation", {FBO(), {0.999}}},
			{"velocity", {velocity_ping, vector<float>()}},
			{"source", {velocity_ping, vector<float>()}},
			{"vector_size", {FBO(), {2.0}}},
			{"dt", {FBO(), {step}}},
		},
		velocity_pong,
		"", false, false
	);
	
	apply_buoyancy = ComputeKernel(
		buoyancy,
		all,
		{
			{"ambient_temperature", {FBO(), {0.0}}},
			{"sigma", {FBO(), {1.5}}}, // Smoke Buoyancy
			{"kappa", {FBO(), {0.05}}}, // Smoke Weight
			{"gravity", {FBO(), {0.0, 0,0}}},
			{"velocity", {velocity_ping, vector<float>()}},
			{"temperature", {temperature_ping, vector<float>()}},
			{"density", {density_ping, vector<float>()}},
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"dt", {FBO(), {step}}},
		},
		velocity_pong,
		"", false, false
	);

	impulse = ComputeKernel(
		add_field,
		all,
		{
			{"px", {FBO(), px}},
			{"source", {density_ping, vector<float>()}},
			{"vector_size", {FBO(), {1.0}}},
			{"force", {FBO(), {0.0, 0.0}}},
			{"center", {FBO(), {10.0, 10.0}}},
			{"scale", {FBO(), {cursor_size * px_x, cursor_size * px_y}}},
		},
		density_pong,
		"", false, false
	);
	
	apply_divergence = ComputeKernel(
		divergence,
		all,
		{
			{"velocity", {velocity_ping, vector<float>()}},
			{"px", {FBO(), px}},
		},
		divergence_ping,
		"", false, false
	);

	compute_jacobi = ComputeKernel(
		jacobi,
		all,
		{
			{"pressure", {pressure_ping, vector<float>()}},
			{"divergence", {divergence_ping, vector<float>()}},
			{"alpha", {FBO(), {-1.0}}},
			{"beta", {FBO(), {0.25}}},
			{"px", {FBO(), px}},
		},
		pressure_pong,
		"", false, false
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
		"", false, false
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
		"", false, false
	);
}


static float x_0 = 0;
static float y_0 = 0;
static float x_1 = 0;
static float y_1 = 0;
static bool is_cursor_down = false;


void FluidTouch(bool is_down, float x, float y) {
	is_cursor_down = is_down;
	x_1 = x;
	y_1 = y;
}

#define M_PI 3.14159265358979323846

static vector<float> gravity = {0, 0};


void FluidRotate(int angle) {
	angle += 90;
	float theta = (float) angle * M_PI / 180.0;
	gravity = {(float) cos(theta), (float) sin(theta)};
}


void SwapBuffers(FBO *fbo0, FBO *fbo1) {
	FBO swap = *fbo0;
	*fbo0 = *fbo1;
	*fbo1 = swap;
}

void FluidUpdate(float elapsed_time) {
	if (busy)
		return;
	UniformsMap uniforms = {
		{"vector_size", {FBO(), {2.0}}},
		{"source", {velocity_ping, vector<float>()}},
		{"dissipation", {FBO(), {0.999}}},
		{"velocity", {velocity_ping, vector<float>()}}
	};
    advect.SetUniforms(uniforms);
    advect.SetFBO(velocity_pong);
	advect.Run();
	SwapBuffers(&velocity_ping, &velocity_pong);

	uniforms = {
		{"vector_size", {FBO(), {1.0}}},
		{"source", {temperature_ping, vector<float>()}},
		{"dissipation", {FBO(), {0.95}}},
		{"velocity", {velocity_ping, vector<float>()}}
	};
    advect.SetUniforms(uniforms);
    advect.SetFBO(temperature_pong);
	advect.Run();

	uniforms = {
		{"source", {density_ping, vector<float>()}},
		{"dissipation", {FBO(), {0.999}}},
	};
    advect.SetUniforms(uniforms);
    advect.SetFBO(density_pong);
	advect.Run();
	
	uniforms = {
		{"velocity", {velocity_ping, vector<float>()}},
		{"temperature", {temperature_pong, vector<float>()}},
		{"density", {density_pong, vector<float>()}},
		{"gravity", {FBO(), gravity}},
	};
    apply_buoyancy.SetUniforms(uniforms);
    apply_buoyancy.SetFBO(velocity_pong);
	apply_buoyancy.Run();


	float xd = x_1 - x_0;
	float yd = y_1 - y_0;
	x_0 = x_1;
	y_0 = y_1;

	UniformsMap force = {
		{"source", {velocity_pong, vector<float>()}},
		{"force", {FBO(),
			{
				xd * px_x * cursor_size * mouse_force,
				-yd * px_y * cursor_size * mouse_force
			}
		}},
		{"vector_size", {FBO(),
			{
				2.0
			}
		}},
		{"center", {FBO(),
			{
				x_0 * px_x,
				1 - y_0 * px_y
			}
		}},
		{"scale", {FBO(),
			{
				cursor_size * px_x,
				cursor_size * px_y
			}
		}}
	};
	
	float force_min = 7.0;
	float force_avg = (float) fabs(xd) + (float) fabs(yd);

	if (is_cursor_down) {
		force_avg = max(force_min, force_avg);
	} else {
		force_avg = 0;
	}
	
	// more radius, less value
	float size_factor = cursor_size * 0.005;
	
	UniformsMap module_force = {
		{"source", {density_pong, vector<float>()}},
		{"force", {FBO(),
			{
				force_avg * px_x * mouse_force / size_factor,
				force_avg * px_y * mouse_force / size_factor,
			}
		}},
		{"vector_size", {FBO(),
			{
				1.0
			}
		}},
	};

	impulse.SetUniforms(force);
    impulse.SetFBO(velocity_ping);
	impulse.Run();

	impulse.SetUniforms(module_force);
    impulse.SetFBO(density_ping);
	impulse.Run();

	uniforms = {
		{"source", {temperature_pong, vector<float>()}},
	};
    impulse.SetUniforms(uniforms);
    impulse.SetFBO(temperature_ping);
	impulse.Run();

	uniforms = {
		{"velocity", {velocity_ping, vector<float>()}},
	};
    apply_divergence.SetUniforms(uniforms);
    apply_divergence.SetFBO(divergence_ping);
	apply_divergence.Run();
	
	//pressure_ping.Clear(0, 0);
	for(int i = 0; i < iterations; i++) {
		SwapBuffers(&pressure_ping, &pressure_pong);
		uniforms = {
			{"divergence", {divergence_ping, vector<float>()}},
			{"pressure", {pressure_ping, vector<float>()}},
		};
    	compute_jacobi.SetUniforms(uniforms);
    	compute_jacobi.SetFBO(pressure_pong);
		compute_jacobi.Run();
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

