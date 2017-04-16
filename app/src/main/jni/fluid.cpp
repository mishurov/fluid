#include "fluid.h"

using namespace std;

//int iterations = 32;
int iterations = 8;
int mouse_force = 1;
// not used float resolution = 0.5;
float cursor_size = 100;
float step = 1/60;


FBO pressureFBO0;
FBO pressureFBO1;

ComputeKernel advectVelocityKernel;
ComputeKernel velocityBoundaryKernel;
ComputeKernel addForceKernel;
ComputeKernel divergenceKernel;
ComputeKernel jacobiKernel;
ComputeKernel pressureBoundaryKernel;
ComputeKernel subtractPressureGradientKernel;
ComputeKernel subtractPressureGradientBoundaryKernel;
ComputeKernel drawKernel;

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
	
	FBO velocityFBO0(width, height, GL_FLOAT, GL_RGBA);
	FBO velocityFBO1(width, height, GL_FLOAT, GL_RGBA);
	FBO divergenceFBO = FBO(width, height, GL_FLOAT, format);
	pressureFBO0 = FBO(width, height, GL_FLOAT, format);
	pressureFBO1 = FBO(width, height, GL_FLOAT, format);

	Shader kernel_advect("shaders/kernel.vertex", "shaders/advect.frag");
	Shader boundary_advect("shaders/boundary.vertex", "shaders/advect.frag");
	Shader cursor_addForce("shaders/cursor.vertex", "shaders/addForce.frag");
	Shader kernel_divergence("shaders/kernel.vertex", "shaders/divergence.frag");
	Shader kernel_jacobi("shaders/kernel.vertex", "shaders/jacobi.frag");
	Shader boundary_jacobi("shaders/boundary.vertex", "shaders/jacobi.frag");
	Shader kernel_subtractPressureGradient(
		"shaders/kernel.vertex", "shaders/subtractPressureGradient.frag"
	);
	Shader boundary_subtractPressureGradient(
		"shaders/boundary.vertex", "shaders/subtractPressureGradient.frag"
	);
	Shader kernel_visualize("shaders/kernel.vertex", "shaders/visualize.frag");

	advectVelocityKernel = ComputeKernel(
		kernel_advect,
		inside,
		{
			{"px", {FBO(), px}},
			{"px1", {FBO(), px1}},
			{"scale", {FBO(), {1.0}}},
			{"velocity", {velocityFBO0, vector<float>()}},
			{"source", {velocityFBO0, vector<float>()}},
			{"dt", {FBO(), {step}}},
		},
		velocityFBO1,
		"",
		false,
		false
	);

	velocityBoundaryKernel = ComputeKernel(
		boundary_advect,
		boundary,
		{
			{"px", {FBO(), px}},
			{"scale", {FBO(), {-1.0}}},
			{"velocity", {velocityFBO0, vector<float>()}},
			{"source", {velocityFBO0, vector<float>()}},
			{"dt", {FBO(), {step}}},
		},
		velocityFBO1,
		"",
		false,
		false
	);

	/* cursor */
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

	addForceKernel = ComputeKernel(
		cursor_addForce,
		cursor,
		{
			{"px", {FBO(), px}},
			{"force", {FBO(), {0.5, 0.2}}},
			{"center", {FBO(), {0.1, 0.4}}},
			{"scale", {FBO(), {cursor_size * px_x, cursor_size * px_y}}},
		},
		velocityFBO1,
		"add",
		false,
		false
	);

	divergenceKernel = ComputeKernel(
		kernel_divergence,
		all,
		{
			{"velocity", {velocityFBO1, vector<float>()}},
			{"px", {FBO(), px}},
		},
		divergenceFBO,
		"",
		false,
		false
	);

	jacobiKernel = ComputeKernel(
		kernel_jacobi,
		all,
		{
			{"pressure", {pressureFBO0, vector<float>()}},
			{"divergence", {divergenceFBO, vector<float>()}},
			{"alpha", {FBO(), {-1.0}}},
			{"beta", {FBO(), {0.25}}},
			{"px", {FBO(), px}},
		},
		pressureFBO1,
		"",
		false,
		true
	);

	pressureBoundaryKernel = ComputeKernel(
		boundary_jacobi,
		boundary,
		{
			{"pressure", {pressureFBO0, vector<float>()}},
			{"divergence", {divergenceFBO, vector<float>()}},
			{"alpha", {FBO(), {-1.0}}},
			{"beta", {FBO(), {0.25}}},
			{"px", {FBO(), px}},
		},
		pressureFBO1,
		"",
		true,
		true
	);
	
	subtractPressureGradientKernel = ComputeKernel(
		kernel_subtractPressureGradient,
		all,
		{
			{"scale", {FBO(), {1.0}}},
			{"pressure", {pressureFBO0, vector<float>()}},
			{"velocity", {velocityFBO1, vector<float>()}},
			{"px", {FBO(), px}},
		},
		velocityFBO0,
		"",
		false,
		false
	);

	subtractPressureGradientBoundaryKernel = ComputeKernel(
		boundary_subtractPressureGradient,
		boundary,
		{
			{"scale", {FBO(), {-1.0}}},
			{"pressure", {pressureFBO0, vector<float>()}},
			{"velocity", {velocityFBO1, vector<float>()}},
			{"px", {FBO(), px}},
		},
		velocityFBO0,
		"",
		false,
		false
	);

	drawKernel = ComputeKernel(
		kernel_visualize,
		all,
		{
			{"velocity", {velocityFBO0, vector<float>()}},
			{"pressure", {pressureFBO0, vector<float>()}},
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

void FluidUpdate(float elapsed_time) {
	// x1, x1 - cursor position
	
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
}

