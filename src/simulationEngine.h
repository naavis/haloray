#pragma once
#include "opengl/shader.h"
#include "opengl/program.h"
#include "opengl/texture.h"

namespace HaloSim
{

class SimulationEngine
{
public:
    void Initialize();

private:
    void InitializeShader();
    void InitializeTextures();
    std::unique_ptr<OpenGL::Program> mSimulationShader;
    std::unique_ptr<OpenGL::Texture> mSimulationTexture;
    std::unique_ptr<OpenGL::Texture> mSpinlockTexture;
};

} // namespace HaloSim
