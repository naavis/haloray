#include "texture.h"
#include <glad/glad.h>
#include <stdexcept>

namespace OpenGL
{

Texture::Texture(unsigned int width, unsigned int height, unsigned int textureUnit, TextureType type)
    : mWidth(width), mHeight(height), mTextureUnit(textureUnit), mType(type)
{
    glGenTextures(1, &mTextureHandle);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, mTextureHandle);
    InitializeTextureImage();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::InitializeTextureImage()
{
    switch (mType)
    {
    case Color:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        break;
    case Monochrome:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, mWidth, mHeight, 0, GL_RED, GL_UNSIGNED_INT, NULL);
        break;
    default:
        throw std::runtime_error("Invalid texture type");
    }
}

Texture::~Texture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &mTextureHandle);
}

const unsigned int Texture::GetHandle() const
{
    return mTextureHandle;
}

const unsigned int Texture::GetTextureUnit() const
{
    return mTextureUnit;
}

} // namespace OpenGL
