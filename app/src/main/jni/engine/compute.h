#ifndef __ENGINE_COMPUTE_H__
#define __ENGINE_COMPUTE_H__

#include "shader.h"
#include "geometry.h"
#include "texture.h"

using namespace std;

class ComputeKernel {
	Shader shader_;
	Mesh mesh_;
	string blend_;
	bool nobind_;
	bool nounbind_;
public:
	/* temp */
	UniformsMap uniforms_;
	FBO outputFBO_;
	ComputeKernel();
	ComputeKernel(
		Shader shader,
		Mesh mesh,
		UniformsMap uniforms,
		FBO outputFBO,
		string blend,
		bool nobind,
		bool nounbind
	);
	void Run();
	void SetUniforms(const UniformsMap& uniforms);
	void SetFBO(FBO fbo);
	void SetMesh(Mesh mesh);
	void SwapBuffer(string fbo_name);
};

#endif // __ENGINE_COMPUTE_H__

