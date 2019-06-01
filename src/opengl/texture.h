#pragma once

namespace OpenGL
{

enum TextureType
{
    Color,
    Monochrome
};

class Texture
{
public:
    Texture(unsigned int width, unsigned int height, unsigned int textureUnit, TextureType type);
    ~Texture();

    const unsigned int GetHandle() const;
    const unsigned int GetTextureUnit() const;

private:
    Texture operator=(const Texture &);
    Texture(const Texture &);
    void InitializeTextureImage();

    unsigned int mTextureHandle;
    unsigned int mWidth;
    unsigned int mHeight;
    unsigned int mTextureUnit;
    unsigned int mType;
};

} // namespace OpenGL
