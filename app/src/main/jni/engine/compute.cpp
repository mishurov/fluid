#include "compute.h"

using namespace std;

ComputeKernel::ComputeKernel() {
}

ComputeKernel::ComputeKernel(
  Shader shader,
  Mesh mesh,
  UniformsMap uniforms,
  FBO outputFBO,
  string blend,
  bool nobind,
  bool nounbind) {
	shader_ = shader;
	mesh_ = mesh;
	uniforms_ = uniforms;
	outputFBO_ = outputFBO;
	blend_ = blend;
	nobind_ = nobind;
	nounbind_ = nounbind;
}

void ComputeKernel::Run() {
	if (outputFBO_.initialized() && !nobind_) {
		outputFBO_.Bind();
	}

	int texture_unit = 0;
	for (UniformsMap::iterator it = uniforms_.begin();
	  it != uniforms_.end();
	  ++it) {
	  	FBO *fbo = &(*it).second.first;
	  	if(fbo->initialized() && !fbo->bound()) {
			fbo->BindTexture(texture_unit++);
		}
	}
	shader_.Use();
    shader_.Uniforms(uniforms_);

	if (blend_ == "add"){
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}

	mesh_.Draw(shader_);

	if (outputFBO_.initialized() && !nounbind_) {
		outputFBO_.Unbind();
	}

	for (UniformsMap::iterator it = uniforms_.begin();
	  it != uniforms_.end();
	  ++it) {
	  	FBO *fbo = &(*it).second.first;
	  	if(fbo->initialized() && fbo->bound()) {
			fbo->UnbindTexture();
		}
	}
}

void ComputeKernel::SetUniforms(const UniformsMap& uniforms) {
	for (UniformsMap::const_iterator it = uniforms.begin();
	  it != uniforms.end();
	  ++it) {
		uniforms_[it->first] = it->second;
	}
}

void ComputeKernel::SetFBO(FBO fbo) {
	outputFBO_ = fbo;
}

void ComputeKernel::SetMesh(Mesh mesh) {
	mesh_ = mesh;
}

void ComputeKernel::SwapBuffer(string fbo_name) {
	for (UniformsMap::iterator it = uniforms_.begin();
	  it != uniforms_.end();
	  ++it) {
	  	if (it->first == fbo_name) {
			FBO swap = outputFBO_;
			outputFBO_ = it->second.first;
			it->second.first = swap;
		}
	}
}
