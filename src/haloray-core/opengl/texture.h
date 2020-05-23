#pragma once
#include <QOpenGLFunctions_4_4_Core>

namespace OpenGL
{

enum TextureType
{
    Color,
    Monochrome
};

class Texture : protected QOpenGLFunctions_4_4_Core
{
public:
    Texture(unsigned int width, unsigned int height, unsigned int textureUnit, TextureType type);
    ~Texture();

    unsigned int getHandle() const;
    unsigned int getTextureUnit() const;

private:
    Texture operator=(const Texture &);
    Texture(const Texture &);
    void initializeTextureImage();

    unsigned int mTextureHandle;
    unsigned int mWidth;
    unsigned int mHeight;
    unsigned int mTextureUnit;
    unsigned int mType;
};

} // namespace OpenGL
