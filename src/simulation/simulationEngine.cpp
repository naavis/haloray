#include "simulationEngine.h"
#include <glad/glad.h>
#include <random>
#include <limits>
#include "../opengl/program.h"
#include "../opengl/texture.h"

#include "../shaders/raytrace.glsl"

namespace HaloSim
{

SimulationEngine::SimulationEngine()
    : mMersenneTwister(std::mt19937(std::random_device()()))
{
}

Camera SimulationEngine::GetCamera() const
{
    return mCamera;
}

void SimulationEngine::SetCamera(struct Camera camera)
{
    mCamera = camera;
}

CrystalPopulation SimulationEngine::GetCrystalPopulation() const
{
    return mCrystals;
}

void SimulationEngine::SetCrystalPopulation(CrystalPopulation crystals)
{
    if (crystals != mCrystals)
        Clear();
    mCrystals = crystals;
}

LightSource SimulationEngine::GetLightSource() const
{
    return mLight;
}

void SimulationEngine::SetLightSource(LightSource light)
{
    if (light != mLight)
        Clear();
    mLight = light;
}

const unsigned int SimulationEngine::GetOutputTextureHandle() const
{
    return mSimulationTexture->GetHandle();
}

void SimulationEngine::Run(unsigned int numRays)
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(mSimulationTexture->GetTextureUnit(), mSimulationTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(mSpinlockTexture->GetHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(mSpinlockTexture->GetTextureUnit(), mSpinlockTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    mSimulationShader->Use();

    GLuint shaderHandle = mSimulationShader->GetHandle();

    std::uniform_int_distribution<unsigned int> distribution(0, std::numeric_limits<unsigned int>::max());
    const unsigned int seed = distribution(mMersenneTwister);
    glUniform1ui(glGetUniformLocation(shaderHandle, "rngSeed"), seed);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.altitude"), mLight.altitude);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.azimuth"), mLight.azimuth);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.diameter"), mLight.diameter);

    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioAverage"), mCrystals.caRatioAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioStd"), mCrystals.caRatioStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleDistribution"), mCrystals.polarAngleDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleAverage"), mCrystals.polarAngleAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.polarAngleStd"), mCrystals.polarAngleStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.rotationDistribution"), mCrystals.rotationDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationAverage"), mCrystals.rotationAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationStd"), mCrystals.rotationStd);

    glUniform1f(glGetUniformLocation(shaderHandle, "camera.pitch"), mCamera.pitch);
    glUniform1f(glGetUniformLocation(shaderHandle, "camera.yaw"), mCamera.yaw);

    glDispatchCompute(numRays, 1, 1);
}

void SimulationEngine::Clear()
{
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glClearTexImage(mSimulationTexture->GetHandle(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(mSimulationTexture->GetTextureUnit(), mSimulationTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(mSpinlockTexture->GetHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(mSpinlockTexture->GetTextureUnit(), mSpinlockTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
}

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