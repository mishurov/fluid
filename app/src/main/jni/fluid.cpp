#include "fluid.h"

using namespace std;

//int iterations = 32;
int iterations = 8;
int mouse_force = 1;
// not used float resolution = 0.5;
float cursor_size = 100;
float step = 1.0 / 60.0;


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
	//vector<float> px1 = { 1.0, (float) width / (float) height };
	vector<float> px1 = { (float) width / (float) height, 1.0 };
	//vector<float> px1 = { 1.0, 1.0 };
	
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

	velocity_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	velocity_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	density_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	density_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	pressure_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	pressure_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	temperature_ping = FBO(width, height, GL_FLOAT, GL_RGBA);
	temperature_pong = FBO(width, height, GL_FLOAT, GL_RGBA);
	divergence_pong = FBO(width, height, GL_FLOAT, GL_RGBA);

	velocity_ping.Clear(0, 1);
	velocity_pong.Clear(0, 1);

	advect_velocity = ComputeKernel(
		advect,
		inside,
		{
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"dissipation", {FBO(), {1.0}}},
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
			{"dissipation", {FBO(), {1.0}}},
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
			{"sigma", {FBO(), {0.7}}}, // Smoke Buoyancy
			{"kappa", {FBO(), {0.05}}}, // Smoke Weight
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

	apply_impulse_temperature = ComputeKernel(
		impulse,
		all,
		{
			{"px", {FBO(), px}},
			{"radius", {FBO(), {0.1}}},
			{"point", {FBO(), {0.5, 0.1}}},
			{"fill_color", {FBO(), {0.01, 0.01, 0.01}}},
		},
		temperature_ping,
		"add",
		false,
		false
	);

	apply_impulse_density = ComputeKernel(
		impulse,
		all,
		{
			{"px", {FBO(), px}},
			{"radius", {FBO(), {0.1}}},
			{"point", {FBO(), {0.5, 0.1}}},
			{"fill_color", {FBO(), {0.04, 0.04, 0.04}}},
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
			{"fill_color", {FBO(), {0.16, 0.01, 0.36}}},
			{"px", {FBO(), px}},
		},
		FBO(),
		"",
		false,
		false
	);
}

float x0 = 0;
float y0 = 0;
float x1 = 0;
float y1 = 0;

void FluidTouch(float x, float y) {
	x1 = x;
	y1 = y;
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
	};
    apply_buoyancy.SetUniforms(uniforms);
    apply_buoyancy.SetFBO(velocity_pong);
	//apply_buoyancy.Run();
	SwapBuffers(&velocity_ping, &velocity_pong);

    apply_impulse_temperature.SetFBO(velocity_ping);
	apply_impulse_temperature.Run();
    apply_impulse_density.SetFBO(density_ping);
	apply_impulse_density.Run();

	uniforms = {
		{"velocity", {velocity_ping, vector<float>()}},
	};
    apply_divergence.SetUniforms(uniforms);
    apply_divergence.SetFBO(divergence_pong);
	apply_divergence.Run();
	
	pressure_ping.Clear(0, 0);
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
		{"sampler", {density_ping, vector<float>()}},
	};
    draw.SetUniforms(uniforms);
	draw.Run();


	/*
	
	float xd = x1 - x0;
 	float yd = y1 - y0;
	x0 = x1;
	y0 = y1;
	
	UniformsMap advect = {{"dt", {FBO(), {step}}}};
    advectVelocityKernel.SetUniforms(advect);
    advectVelocityKernel.Run();

	UniformsMap force = {
		{"force", {FBO(),
			{
				xd * px_x * cursor_size * mouse_force,
				-yd * px_y * cursor_size * mouse_force
			}
		}},
		{"center", {FBO(),
			{
				x0 * px_x * 2 - 1,
				(y0 * px_y * 2 - 1) * -1
			}
		}}
	};
    addForceKernel.SetUniforms(force);

    addForceKernel.Run();
	velocityBoundaryKernel.Run();
	divergenceKernel.Run();
	
	FBO p0 = pressureFBO0;
	FBO p1 = pressureFBO1;
	FBO p_ = p0;
	for(int i = 0; i < iterations; i++) {
		UniformsMap pressure = {{"pressure", {p0, vector<float>()}}};
    	jacobiKernel.SetUniforms(pressure);
    	jacobiKernel.SetFBO(p1);
		jacobiKernel.Run();
		pressureBoundaryKernel.Run();
		p_ = p0;
		p0 = p1;
		p1 = p_;
	}

	subtractPressureGradientKernel.Run();
    subtractPressureGradientBoundaryKernel.Run();
	
	drawKernel.Run();
	*/
}

