#include "texture.h"
#include <stdexcept>

namespace OpenGL
{

Texture::Texture(unsigned int width, unsigned int height, unsigned int textureUnit, TextureType type)
    : m_width(width), m_height(height), m_textureUnit(textureUnit), m_type(type)
{
    initializeOpenGLFunctions();
    glGenTextures(1, &m_textureHandle);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    initializeTextureImage();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::initializeTextureImage()
{
    switch (m_type)
    {
    case Color:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        break;
    case Monochrome:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_width, m_height, 0, GL_RED, GL_FLOAT, NULL);
        break;
    default:
        throw std::runtime_error("Invalid texture type");
    }
}

Texture::~Texture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &m_textureHandle);
}

unsigned int Texture::getHandle() const
{
    return m_textureHandle;
}

unsigned int Texture::getTextureUnit() const
{
    return m_textureUnit;
}

}
