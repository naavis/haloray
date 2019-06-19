#include "program.h"
#include <stdexcept>

namespace OpenGL
{

Program::Program()
{
    initializeOpenGLFunctions();
    mProgramHandle = glCreateProgram();
}

Program::~Program()
{
    glUseProgram(0);
    glDeleteProgram(mProgramHandle);
}

void Program::AttachShader(const Shader &shader)
{
    glAttachShader(mProgramHandle, shader.GetHandle());
}

void Program::Link()
{
    glLinkProgram(mProgramHandle);
    mIsLinked = true;
}

void Program::Use()
{
    if (!mIsLinked)
        throw std::runtime_error("Program is not linked yet");
    glUseProgram(mProgramHandle);
}

const unsigned int Program::GetHandle() const
{
    if (mIsLinked)
        return mProgramHandle;
    throw std::runtime_error("Program is not linked yet");
}

} // namespace OpenGL
