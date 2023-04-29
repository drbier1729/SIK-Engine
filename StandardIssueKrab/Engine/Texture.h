#pragma once

/*
Encapsulation of an OpenGL Texture
Loads a texture from a png using stb_image and then stores it on the graphics card
Can bind/unbind a texture with a shader program
*/
class Texture {

public:
    Uint32 texture_id = 0;
    Int32 width = 0, height = 0;
    Int32 channel_count = 0;

    // for model and skinned mesh
    String texture_path;
    String texture_type;

    // Null-constructor. Doesn't do anything, but needed to allow for
    // texture loading.
    Texture() = default;

    // Constructs a texture from pre-loaded data. Data can be freed once
    // this function returns.
    explicit Texture(Int32 w, Int32 h, Int32 chs, unsigned char* data);

    // Non-copyable
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    ~Texture();


    void Bind(const Uint32 unit, const Uint32 program_id, const char* name) const;
    void Unbind() const;
    void GenerateDefaultMipmap() const;


private:
    static Uint32 default_id;

private:
    static Uint32 GenTexture(GLuint min_filter, GLuint mag_filter,
        Int32 width, Int32 height, Int32 channel_count, unsigned char* data);

public:
    static void InitDefault();
    static void FreeDefault();
};
