#ifndef __ENGINE_GEOMETRY_H__
#define __ENGINE_GEOMETRY_H__

#include "shader.h"

using namespace std;

typedef map<string, map<string, int> > AttributesMap;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

vector<float> ScreenQuad(float xscale, float yscale);


class Buffer {
	GLuint buffer_;
	GLenum target_;
	int length_;

	void Free();

public:
	Buffer();
	Buffer(const vector<float>& data);
	void Bind();
	void Unbind();
	int length();
};


class Mesh {
	GLenum mode_;
	AttributesMap attributes_;
	Buffer vbo_;
	GLuint gl_ibo_;
	int vertex_size_;
	void SetAttributes(const AttributesMap& attributes);
	void BindAttributes(Shader& shader);
public:
	Mesh();
	Mesh(
	  GLenum mode, 
	  const vector<float>& vertex,
	  const vector<unsigned short>& index,
	  const AttributesMap& attributes
	);
	void Draw(Shader& shader);
};

#endif // __ENGINE_GEOMETRY_H__
