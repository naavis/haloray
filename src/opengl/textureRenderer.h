#pragma once
#include <memory>
#include <string>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_4_Core>

namespace OpenGL
{
class TextureRenderer : protected QOpenGLFunctions_4_4_Core
{
public:
    TextureRenderer() = default;
    void Initialize();
    void SetUniformFloat(std::string name, float value);
    void Render(unsigned int textureHandle);
    ~TextureRenderer();

private:
    TextureRenderer(const TextureRenderer &) = default;
    TextureRenderer &operator=(const TextureRenderer &) = default;

    static std::unique_ptr<QOpenGLShaderProgram> InitializeTexDrawShaderProgram();
    std::unique_ptr<QOpenGLShaderProgram> mTexDrawProgram;
    unsigned int mQuadVao;
    unsigned int mQuadVbo;
};
} // namespace OpenGL
