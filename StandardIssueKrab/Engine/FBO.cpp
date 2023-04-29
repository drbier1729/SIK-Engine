#include "stdafx.h"
#include "FBO.h"

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error in FBO.cpp:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }

/*Constructor for the FBO object
* Creates an FBO with a specified width, height and
* number of color_attachments
*/
FBO::FBO(const Uint32 w, const Uint32 h, const Uint8 _color_attachment_count, Bool _has_depth_buffer) :
    width(w), height(h), color_attachment_count(_color_attachment_count), 
    has_depth_buffer(_has_depth_buffer) {
    glGenFramebuffersEXT(1, &fboID);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);

    if (has_depth_buffer) {
        // Create a render buffer, and attach it to FBO's depth attachment
        glGenRenderbuffersEXT(1, &depthBufferID);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBufferID);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
            width, height);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
            GL_RENDERBUFFER_EXT, depthBufferID);
    }

    SIK_ASSERT(color_attachment_count <= max_attachment_count, "Exceeded maximum color attachments");

    // Create a texture and attach FBO's color 0 attachment.  The
    // GL_RGBA32F and GL_RGBA constants set this texture to be 32 bit
    // floats for each of the 4 components.  Many other choices are
    // possible.
    for (Uint32 i = 0; i < color_attachment_count; ++i) {
        glGenTextures(1, &textureID[i]);
        glBindTexture(GL_TEXTURE_2D, textureID[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, (GLenum)((int)GL_COLOR_ATTACHMENT0_EXT + i),
            GL_TEXTURE_2D, textureID[i], 0);
    }

    // Check for completeness/correctness
    int status = (int)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    SIK_ASSERT(status == int(GL_FRAMEBUFFER_COMPLETE_EXT), "FBO Creation error");

    // Unbind the fbo until it's ready to be used
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

FBO::~FBO() {
    if (fboID == 0)
        return;

    glDeleteTextures(color_attachment_count, textureID);
    glDeleteRenderbuffers(1, &depthBufferID);
    glDeleteFramebuffers(1, &fboID);
}

void FBO::Bind() const { 
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID); 
    glViewport(0, 0, GetWidth(), GetHeight());
    CHECKERROR
}
void FBO::Unbind() const { glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); }

void FBO::BindTexture(const Uint8 program_id,const char* var_name, const Uint8 color_attachment) const {
    glActiveTexture((GLenum)(int)GL_TEXTURE0 + base_texture_unit + color_attachment);
    glBindTexture(GL_TEXTURE_2D, textureID[color_attachment]); // Load texture into it
    int loc = glGetUniformLocation(program_id, var_name);
    glUniform1i(loc, base_texture_unit + color_attachment);
}

void FBO::UnbindTexture(const Uint8 color_attachment) const {
    glActiveTexture((GLenum)(int)GL_TEXTURE1 + color_attachment);
    glBindTexture(GL_TEXTURE_2D, 0); // Load texture into it
}

void FBO::BindTexture(const Uint8 program_id, const char* var_name, const Uint8 color_attachment, const Uint8 texture_unit) const {
    glActiveTexture((GLenum)(int)(GL_TEXTURE0 + texture_unit));
    glBindTexture(GL_TEXTURE_2D, textureID[color_attachment]); // Load texture into it
    int loc = glGetUniformLocation(program_id, var_name);
    glUniform1i(loc, texture_unit);
}

void FBO::BindImageTexture(const Uint8 program_id, const char* var_name, const Uint8 color_attachment,
    const Uint8 texture_unit, const Uint8 mip_level) const {
    Uint8 loc = glGetUniformLocation(program_id, var_name);
    glBindImageTexture(texture_unit, textureID[color_attachment],
        mip_level, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    CHECKERROR;
    glUniform1i(loc, texture_unit);
}

void FBO::UnbindTexture(const Uint8 /*color_attachment*/, const Uint8 texture_unit) const {
    glActiveTexture((GLenum)(int)(GL_TEXTURE0 + texture_unit));
    glBindTexture(GL_TEXTURE_2D, 0); // Load texture into it
}

Uint32 FBO::GetWidth() const {
    return width;
}

Uint32 FBO::GetHeight() const {
    return height;
}

void FBO::BindDrawFBO() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);    
}

void FBO::UnbindDrawFBO() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}