#include "simulationEngine.h"
#include <glad/glad.h>
#include "opengl/program.h"
#include "opengl/texture.h"

const std::string computeShaderSource =
#include "shaders/raytrace.glsl"
    ;

namespace HaloSim
{

void SimulationEngine::Initialize()
{
    InitializeShader();
    InitializeTextures();
}

void SimulationEngine::InitializeShader()
{
    OpenGL::Shader shader(computeShaderSource, OpenGL::ShaderType::Compute);
    mSimulationShader = std::make_unique<OpenGL::Program>();
    try
    {
        shader.Compile();
        mSimulationShader->AttachShader(shader);
        mSimulationShader->Link();
    }
    catch (const std::runtime_error &)
    {
        throw;
    }
}

void SimulationEngine::InitializeTextures()
{
    mSimulationTexture = std::make_unique<OpenGL::Texture>(1920, 1080, 0, OpenGL::TextureType::Color);
    mSpinlockTexture = std::make_unique<OpenGL::Texture>(1920, 1080, 1, OpenGL::TextureType::Monochrome);
}

} // namespace HaloSim
