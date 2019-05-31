#include "program.h"
#include <glad/glad.h>
#include "utils.h"

namespace OpenGL
{

Program::Program()
{
    mProgramHandle = glCreateProgram();
}

Program::~Program()
{
    glDeleteProgram(mProgramHandle);
}

void Program::AttachShader(const Shader &shader) const
{
    glAttachShader(mProgramHandle, shader.GetShaderHandle());
}

void Program::Link()
{
    glLinkProgram(mProgramHandle);
    CheckLinkError(mProgramHandle);
    mIsLinked = true;
}

unsigned int Program::GetProgramHandle() const
{
    if (mIsLinked)
        return mProgramHandle;
    throw std::runtime_error("Program is not linked yet");
}

} // namespace OpenGL
