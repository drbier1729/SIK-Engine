#pragma once
class FBO {
private:
    Uint32 width, height;  // Size of the texture.
    GLuint color_attachment_count;
    static constexpr int base_texture_unit = 1;
    static constexpr Uint8 max_attachment_count = 5;
    Bool has_depth_buffer;
public:
    GLuint fboID = 0;
    GLuint depthBufferID = 0;
    GLuint textureID[max_attachment_count] = { 0, 0, 0, 0 };
    /*Constructor for the FBO object
    * Creates an FBO with a specified width, height and
    * number of color_attachments
    * Set _has_depth_buffer to false if you don't need z-tests on the buffer
    */
    FBO(const Uint32 w, const Uint32 h, const Uint8 _color_attachment_count = 1, Bool _has_depth_buffer = true);
    /*Deletes an FBO by deleting the associated texture
    * and frame buffer
    */
    ~FBO();
    /* Bind this FBO so it's used as the render target for the next draw call
    * Returns: void
    */
    void Bind() const;
    /* UnBind this FBO so that the screen is the render target for the next
    *  draw call
    * Returns: void
    */
    void Unbind() const;
    /* Bind this FBOs texture to a texture unit so it can be accessed by
    * the shader.
    * Returns: void
    */
    void BindTexture(const Uint8 program_id, const char* var_name, const Uint8 color_attachment = 0) const;
    /* Unbind a texture from a texture
    * Returns: void
    */
    void UnbindTexture(const Uint8 color_attachment) const;
    /* Bind this FBOs texture to a texture unit so it can be accessed by
    * the shader.
    * Returns: void
    */
    void BindTexture(const Uint8 program_id, const char* var_name, const Uint8 color_attachment, const Uint8 texture_unit) const;
    /* Bind this FBOs texture to a texture unit  as an image 
    * so it can be accessed by a compute shader.
    * Returns: void
    */
    void BindImageTexture(const Uint8 program_id, const char* var_name, const Uint8 color_attachment, const Uint8 texture_unit, const Uint8 mip_level) const;
    /* Unbind a texture from a texture
    * Returns: void
    */
    void UnbindTexture(const Uint8 color_attachment, const Uint8 texture_unit) const;
    /* Provides width of FBO
    * Returns: Uint32 width of FBO in pixels
    */
    Uint32 GetWidth() const;
    /* Provides height of FBO
   * Returns: Uint32 height of FBO in pixels
   */
    Uint32 GetHeight() const;
    /* Bind this FBO as GL_DRAW_FRAMEBUFFER
    *  Returns void
    */
    void BindDrawFBO();
    /* Unbind this GL_DRAW_FRAMEBUFFER
    *  Returns void
    */
    void UnbindDrawFBO();
};

