#pragma once
#include "shader.h"

namespace OpenGL
{
class Program
{
public:
    Program();
    ~Program();
    void AttachShader(const Shader &shader) const;
    void Link();
    void Use();

    unsigned int GetProgramHandle() const;

private:
    Program(const Program &);
    Program operator=(const Program &);
    unsigned int mProgramHandle;
    bool mIsLinked;
};
} // namespace OpenGL
