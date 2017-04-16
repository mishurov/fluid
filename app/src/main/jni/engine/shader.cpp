#include "shader.h"

using namespace std;

Shader::Shader() {
};

Shader::Shader(const char* vertex_path, const char* fragment_path) {
	program_ = CreateProgram(vertex_path, fragment_path);
};

void Shader::Use() {
	glUseProgram(program_);
}

void Shader::PrepareUniforms(const UniformsMap& values) {
	for (UniformsMap::const_iterator it = values.begin();
	  it != values.end();
	  ++it) {
		uniform_names_.push_back(it->first);
		uniform_locations_[it->first] =
			glGetUniformLocation(program_, it->first.c_str());
	}
}

void Shader::Uniforms(const UniformsMap& values) {
	if (!uniform_names_.size()) {
		PrepareUniforms(values);
	}
	for (vector<string>::const_iterator it = uniform_names_.begin();
	  it != uniform_names_.end();
	  ++it) {

		string name = *it;
		GLuint location = uniform_locations_.at(name);
	
		FBO fbo = values.at(name).first;
		vector<float> value = values.at(name).second;
		
		//__android_log_print(ANDROID_LOG_INFO,"Fluid", "exception");
		try {
			uniform_values_.at(name);
		}
		catch (const out_of_range& oor) {
			uniform_values_[name] = { FBO(), vector<float>() };
		}
		
		if (fbo.initialized()) {
			if(!fbo.Equals(uniform_values_.at(name).first.unit())) {
				fbo.Uniform(location);
				uniform_values_.at(name).first = fbo;
			}
		} else {
			uniform_values_.at(name).first = FBO();
			switch (value.size()) {
				case 1:
					glUniform1f(location, value.at(0));
					uniform_values_.at(name).second = value;
					break;
				case 2: 
					glUniform2f(location, value.at(0), value.at(1));
					uniform_values_.at(name).second = value;
					break;
				case 3: 
					glUniform3f(location, value.at(0), value.at(1), value.at(2));
					uniform_values_.at(name).second = value;
					break;
				case 4: 
					glUniform4f(
						location, value.at(0), value.at(1),value.at(2), value.at(3) 
					);
					uniform_values_.at(name).second = value;
					break;
				case 9: 
					glUniformMatrix3fv(location, GL_FALSE, 1, &value[0]);
					uniform_values_.at(name).second = value;
					break;
				case 16: 
					glUniformMatrix4fv(location, GL_FALSE, 1, &value[0]);
					uniform_values_.at(name).second = value;
					break;
			}
		}
	}
}

GLuint Shader::GetUniformLocation(string name) {
	GLuint ret;
	try {
		ret = uniform_locations_.at(name);
	}
	catch (const out_of_range& oor) {
		uniform_locations_[name] = glGetUniformLocation(program_, name.c_str());
		ret = uniform_locations_.at(name);
	}
	return ret;
}

GLuint Shader::GetAttribLocation(string name) {
	GLuint ret;
	try {
		ret = attribute_locations_.at(name);
	}
	catch (const out_of_range& oor) {
		attribute_locations_[name] = glGetAttribLocation(program_, name.c_str());
		ret = attribute_locations_.at(name);
	}
	return ret;
}

