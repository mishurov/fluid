#ifndef __ENGINE_SHADER_H__
#define __ENGINE_SHADER_H__

#include <string>
#include <map>
#include <vector>
#include "../utils/shader.h"
#include "texture.h"

#include <android/log.h>

using namespace std;

typedef map<string, pair<FBO, vector<float> > > UniformsMap;

class Shader {
	GLuint program_;
	vector<string> uniform_names_;
	map<string,GLuint> uniform_locations_;
	UniformsMap uniform_values_;
	map<string,GLuint> attribute_locations_;

	void PrepareUniforms(const UniformsMap& values);
	GLuint GetUniformLocation(string name);

public:
	Shader();
	Shader(const char* vertex_path, const char* fragment_path);
	void Use();
	void Uniforms(const UniformsMap& values);
	GLuint GetAttribLocation(string name);
};

#endif // __ENGINE_SHADER_H__
