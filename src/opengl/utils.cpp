#include "utils.h"
#include <glad/glad.h>
#include <string>
#include <stdexcept>

namespace OpenGL
{
void CheckCompileError(unsigned int shaderHandle)
{
    int rvalue;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue)
    {
        GLchar log[10240];
        GLsizei length;
        glGetShaderInfoLog(shaderHandle, 10239, &length, log);
        throw std::runtime_error(log);
    }
}

void CheckLinkError(unsigned int programHandle)
{
    int rvalue;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue)
    {
        GLchar log[10240];
        GLsizei length;
        glGetProgramInfoLog(programHandle, 10239, &length, log);
        throw std::runtime_error(log);
    }
}
} // namespace OpenGL
