#include "shader.h"
#include "utils.h"
#include <stdexcept>
#include <string>
#include <glad/glad.h>

namespace OpenGL
{

Shader::Shader(std::string source, ShaderType type) : mSource(source), mType(type)
{
    mShaderHandle = glCreateShader((GLenum)type);
}

void Shader::Compile()
{
    const GLchar *src = mSource.c_str();
    glShaderSource(mShaderHandle, 1, &src, NULL);
    glCompileShader(mShaderHandle);
    CheckCompileError(mShaderHandle);
    mIsCompiled = true;
}

Shader::~Shader()
{
    glDeleteShader(mShaderHandle);
}

const unsigned int Shader::GetHandle() const
{
    if (mIsCompiled)
        return mShaderHandle;
    throw std::runtime_error("Shader is not compiled yet");
}

} // namespace OpenGL
