#pragma once
#include <memory>
#include <string>
#include <QOpenGLFunctions_4_4_Core>
#include <QOpenGLShaderProgram>

namespace OpenGL
{
class TextureRenderer : protected QOpenGLFunctions_4_4_Core
{
public:
    TextureRenderer() = default;
    void initialize();
    void setUniformFloat(std::string name, float value);
    void render(unsigned int textureHandle);
    ~TextureRenderer();

private:
    static std::unique_ptr<QOpenGLShaderProgram> initializeTexDrawShaderProgram();

    std::unique_ptr<QOpenGLShaderProgram> m_texDrawProgram;
    unsigned int m_quadVao;
    unsigned int m_quadVbo;
};
}
