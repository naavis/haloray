#pragma once
#include <memory>
#include <string>
#include "program.h"

namespace OpenGL
{
class TextureRenderer
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

    static std::unique_ptr<OpenGL::Program> InitializeTexDrawShaderProgram();
    std::unique_ptr<OpenGL::Program> mTexDrawProgram;
    unsigned int mQuadVao;
    unsigned int mQuadVbo;
};
} // namespace OpenGL
