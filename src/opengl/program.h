#pragma once
#include "shader.h"
#include <QOpenGLFunctions_4_4_Core>

namespace OpenGL
{
class Program : protected QOpenGLFunctions_4_4_Core
{
public:
    Program();
    ~Program();
    void AttachShader(const Shader &shader);
    void Link();
    void Use();

    const unsigned int GetHandle() const;

private:
    Program(const Program &);
    Program operator=(const Program &);
    unsigned int mProgramHandle;
    bool mIsLinked;
};
} // namespace OpenGL
