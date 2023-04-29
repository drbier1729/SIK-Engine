#pragma once

class FBO;
class MSAA {
private:
    GLuint fboID;

    GLsizei width;
    GLsizei height;

    void BindReadFBO();
    void UnbindReadFBO();

public:
    MSAA();    
    MSAA(GLsizei const samples, GLsizei const _width, GLsizei const _height);
    ~MSAA() = default;

    void Init(GLsizei const& _samples, GLsizei const& _width, GLsizei const& _height);
    void Bind();
    void Unbind();
    void Draw(FBO* drawFBO);
};