#include <regex>
#include "shader.h"

using namespace std;

Shader::Shader() {
	initialized_ = false;
};

Shader::Shader(string vertex_path, string fragment_path) {
	program_ = CreateProgram(vertex_path, fragment_path);
	initialized_ = true;
};

void Shader::Use() {
	glUseProgram(program_);
}

void Shader::PrepareUniforms(const UniformsMap& values) {
	uniform_names_ = vector<string>();
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

GLuint Shader::CompileShader(string path, GLenum type) {
	
	string shader_prefix = "shaders/";
	char* shader_data_ptr = NULL;
	unsigned int size = 0;

	LogInfo("Compiling shader...");
	LogInfo(path);
	ReadFile(path.c_str(), &shader_data_ptr, &size);
	string shader_data = shader_data_ptr;
	
	regex include_regex("#include \"([^\"]+)\"");
	smatch include_match;
	if (regex_search(shader_data, include_match, include_regex)) {
		for (size_t i = 1; i < include_match.size(); ++i) {
			string include_path = include_match[i].str();
			include_path.insert (0, shader_prefix);
    		char* include_shader_data_ptr = NULL;
    		ReadFile(include_path.c_str(), &include_shader_data_ptr, &size);
			string include_data = include_shader_data_ptr;
			shader_data = regex_replace(shader_data, include_regex, include_data);
		}
	}
	if (type == GL_FRAGMENT_SHADER)
		shader_data.insert(0, "#define FRAGMENT\n");
	else if (type == GL_VERTEX_SHADER)
		shader_data.insert(0, "#define VERTEX\n");
	shader_data.insert(0, "#version 100\n");

	GLuint handle = glCreateShader(type);
	if (handle == 0)
		return 0;
	const char *shader_data_const = shader_data.c_str();
	glShaderSource(handle, 1, &shader_data_const, NULL);

	// Compile the shader
	glCompileShader(handle);

	GLint status;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status );

	if(!status) {
		GLint info_length = 0;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &info_length );

		if (info_length > 1) {
			char* info_log = new char[info_length];
			glGetShaderInfoLog(handle, info_length, NULL, info_log);
			char error_string[1024];
			sprintf(error_string, "Error compiling shader:\n%s\n", info_log); 
			LogError(error_string);
			delete[] info_log;
		}
		glDeleteShader(handle);
		// Failed to compile the shadedrs
		return 0;
	}
	return handle;
}

GLuint Shader::CreateProgram(string vertex_path, string fragment_path) {
	GLuint vertex_handle = CompileShader(vertex_path, GL_VERTEX_SHADER);
	GLuint fragment_handle = CompileShader(fragment_path, GL_FRAGMENT_SHADER);

    GLuint handle = glCreateProgram();
    if (handle == 0) {
        return 0;
    }
    glAttachShader(handle, vertex_handle);
    glAttachShader(handle, fragment_handle);
    glLinkProgram(handle);
	
    GLint status;
    glGetProgramiv(handle, GL_LINK_STATUS, &status );

	if (!status) {
		GLint info_length = 0;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_length);
		if (info_length > 1) {
			char* info_log = new char[info_length];
			glGetProgramInfoLog(handle, info_length, NULL, info_log);
			char error_string[1024];
			sprintf(error_string, "Error linking program:\n%s\n", info_log); 
			LogError(error_string);
			delete[] info_log;
		}
		glDeleteProgram(handle);
		return 0;
	}
	return handle;
}

bool Shader::initialized() {
	return initialized_;
}
