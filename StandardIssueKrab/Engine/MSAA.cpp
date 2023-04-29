#include "stdafx.h"
#include "FBO.h"
#include "MSAA.h"

MSAA::MSAA() : width(800), height(600) {
	GLsizei const samples = 4;

	Init(samples, width, height);
}

MSAA::MSAA(GLsizei const samples, GLsizei const _width, GLsizei const _height) : width(_width), height(_height) {
	Init(samples, width, height);
}

void MSAA::Init(GLsizei const& samples, GLsizei const& width, GLsizei const& height) {
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	GLuint MsaaTex;
	glGenTextures(1, &MsaaTex);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, MsaaTex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, MsaaTex, 0);


	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		SIK_ERROR("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MSAA::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glViewport(0, 0, width, height);
}

void MSAA::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MSAA::BindReadFBO() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID);
}

void MSAA::UnbindReadFBO() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void MSAA::Draw(FBO* drawFBO) {
	BindReadFBO();
	drawFBO->BindDrawFBO();

	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	drawFBO->UnbindDrawFBO();
	UnbindReadFBO();
}