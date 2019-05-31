#pragma once
#include "shader.h"

namespace OpenGL
{
class Program
{
public:
    Program();
    ~Program();
    Program(const Program &);
    void AttachShader(Shader shader);
    void Link();

    unsigned int GetProgramHandle() const;

private:
    Program operator=(const Program &);
    unsigned int mProgramHandle;
    bool mIsLinked;
};
} // namespace OpenGL
