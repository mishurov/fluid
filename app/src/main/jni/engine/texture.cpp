#include "texture.h"


FBO::FBO() {
	initialized_ = false;
}

FBO::FBO(int width, int height, GLenum type, GLenum format) {
	width_ = width;
	height_ = height;
	
	glGenFramebuffers(1, &framebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);

	glTexImage2D(
		GL_TEXTURE_2D, 0, format, width_, height_, 0, format, type, 0
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenRenderbuffers(1, &depth_);

 	glBindRenderbuffer(GL_RENDERBUFFER, depth_);
    glRenderbufferStorage(
		GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width_, height_
	);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0
	);
    glFramebufferRenderbuffer(
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_
	);

	supported_ = (
		glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
	);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	initialized_ = true;
	unit_ = -1;
}

void FBO::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
}

void FBO::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::BindTexture(int unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
	unit_ = unit;
	glBindTexture(GL_TEXTURE_2D, texture_);
	bound_ = true;
}

void FBO::UnbindTexture() {
	glActiveTexture(GL_TEXTURE0 + unit_);
	glBindTexture(GL_TEXTURE_2D, 0);
	unit_ = -1;
	bound_ = false;
}

bool FBO::bound() {
	return bound_;
}

bool FBO::initialized() {
	return initialized_;
}

bool FBO::supported() {
	return supported_;
}

bool FBO::Equals(int value) {
	return unit_ == value;
}

void FBO::Uniform(GLuint location) {
	glUniform1i(location, unit_);
}

int FBO::unit() {
	return unit_;
}

