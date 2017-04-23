#ifndef __ENGINE_SHADER_H__
#define __ENGINE_SHADER_H__

#include <string>
#include <map>
#include <vector>
#include "../utils/file.h"
#include "log.h"
#include "texture.h"


using namespace std;

typedef map<string, pair<FBO, vector<float> > > UniformsMap;

class Shader {
	GLuint program_;
	vector<string> uniform_names_;
	map<string,GLuint> uniform_locations_;
	UniformsMap uniform_values_;
	map<string,GLuint> attribute_locations_;
	bool initialized_;

	void PrepareUniforms(const UniformsMap& values);
	GLuint GetUniformLocation(string name);
	GLuint CompileShader(string path, GLenum type);
	GLuint CreateProgram(string vertex_path, string fragment_path);
public:
	Shader();
	Shader(string vertex_path, string fragment_path);
	void Use();
	void Uniforms(const UniformsMap& values);
	bool initialized();
	GLuint GetAttribLocation(string name);
};

#endif // __ENGINE_SHADER_H__
