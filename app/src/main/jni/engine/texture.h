#ifndef __ENGINE_TEXTURE_H__
#define __ENGINE_TEXTURE_H__

#include "../utils/opengl.h"


class FBO {
	bool initialized_;
	int width_;
	int height_;
	GLuint framebuffer_;
	GLuint texture_;
	//GLuint depth_;
	int unit_;
	bool bound_;
	bool supported_;
	int value_;

public:
	FBO();
	FBO(int width, int height, GLenum type, GLenum format);
	void Bind();
	void Unbind();
	void BindTexture(int unit);
	void UnbindTexture();
	bool bound();
	bool supported();
	bool initialized();
	bool Equals(int value);
	void Uniform(GLuint location);
	void Set(int value);
	int unit();
	void Clear(float value, float alpha);

};


#endif // __ENGINE_TEXTURE_H__

