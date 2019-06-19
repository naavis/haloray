#pragma once
#include <string>
#include <QOpenGLFunctions_4_4_Core>

namespace OpenGL
{

enum ShaderType
{
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Compute = GL_COMPUTE_SHADER
};

class Shader : protected QOpenGLFunctions_4_4_Core
{
public:
    Shader(std::string source, ShaderType type);
    ~Shader();
    void Compile();
    const unsigned int GetHandle() const;

private:
    Shader(const Shader &);
    Shader operator=(const Shader &);
    std::string mSource;
    ShaderType mType;
    unsigned int mShaderHandle;
    bool mIsCompiled;
};

} // namespace OpenGL
