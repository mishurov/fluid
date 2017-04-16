#include "geometry.h"

using namespace std;


vector<float> ScreenQuad(float xscale, float yscale) {
	return {
		-xscale,  yscale, 0,
		-xscale, -yscale, 0,
		 xscale, -yscale, 0,
		-xscale,  yscale, 0,
		 xscale, -yscale, 0,
		 xscale,  yscale, 0
	};
}

Buffer::Buffer() {
}

Buffer::Buffer(const vector<float>& data) {
	target_ = GL_ARRAY_BUFFER;
	glGenBuffers(1, &buffer_);
	Bind();
	length_ = data.size();
	glBufferData(target_, length_ * sizeof(float), &data[0], GL_STATIC_DRAW);
	Unbind();
}

void Buffer::Bind() {
	glBindBuffer(target_, buffer_);
}

void Buffer::Unbind() {
	glBindBuffer(target_, 0);
}

void Buffer::Free() {
	glDeleteBuffers(1, &buffer_);
	buffer_ = 0;
}

int Buffer::length() {
	return length_;
}

Mesh::Mesh() {
}

Mesh::Mesh(
  GLenum mode,
  const vector<float>& vertices,
  const vector<unsigned short>& indices,
  const AttributesMap& attributes) {
	vertex_size_ = indices.size();
	mode_ = mode;
	vbo_ = Buffer(vertices);
	SetAttributes(attributes);

	/* index buffer */
	glGenBuffers(1, &gl_ibo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ibo_);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned short) * indices.size(),
		&indices[0],
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::SetAttributes(const AttributesMap& attributes) {
	attributes_ = attributes;
}

void Mesh::BindAttributes(Shader& shader) {
	for(AttributesMap::const_iterator it = attributes_.begin();
	  it != attributes_.end();
	  ++it) {
		GLuint location = shader.GetAttribLocation(it->first);
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(
			location,
			it->second.at("size"),
			GL_FLOAT,
			GL_FALSE,
			it->second.at("stride") * sizeof(GL_FLOAT),
			BUFFER_OFFSET(it->second.at("offset"))
		);
		//vertex_size_ += it->second.at("size");
	}
}

void Mesh::Draw(Shader& shader) {
	shader.Use();
	vbo_.Bind();
	BindAttributes(shader);
	//glDrawArrays(mode_, 0, vbo_.length()*3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_ibo_);
	glDrawElements(mode_, vertex_size_, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	vbo_.Unbind();
}

