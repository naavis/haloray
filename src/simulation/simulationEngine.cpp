#include "simulationEngine.h"
#include <glad/glad.h>
#include <memory>
#include <random>
#include <limits>
#include "../opengl/program.h"
#include "../opengl/texture.h"

#include "shaders/raytrace.glsl"

namespace HaloSim
{

SimulationEngine::SimulationEngine(
    unsigned int outputWidth,
    unsigned int outputHeight)
    : mOutputWidth(outputWidth),
      mOutputHeight(outputHeight),
      mMersenneTwister(std::mt19937(std::random_device()())),
      mUniformDistribution(std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max()))
{
}

Camera SimulationEngine::GetCamera() const
{
    return mCamera;
}

void SimulationEngine::SetCamera(const struct Camera camera)
{
    Clear();
    mCamera = camera;
}

CrystalPopulation SimulationEngine::GetCrystalPopulation() const
{
    return mCrystals;
}

void SimulationEngine::SetCrystalPopulation(const CrystalPopulation crystals)
{
    Clear();
    mCrystals = crystals;
}

LightSource SimulationEngine::GetLightSource() const
{
    return mLight;
}

void SimulationEngine::SetLightSource(const LightSource light)
{
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

    const unsigned int seed = mUniformDistribution(mMersenneTwister);
    glUniform1ui(glGetUniformLocation(shaderHandle, "rngSeed"), seed);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.altitude"), mLight.altitude);
    glUniform1f(glGetUniformLocation(shaderHandle, "sun.diameter"), mLight.diameter);

    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioAverage"), mCrystals.caRatioAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.caRatioStd"), mCrystals.caRatioStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.tiltDistribution"), mCrystals.tiltDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.tiltAverage"), mCrystals.tiltAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.tiltStd"), mCrystals.tiltStd);

    glUniform1i(glGetUniformLocation(shaderHandle, "crystalProperties.rotationDistribution"), mCrystals.rotationDistribution);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationAverage"), mCrystals.rotationAverage);
    glUniform1f(glGetUniformLocation(shaderHandle, "crystalProperties.rotationStd"), mCrystals.rotationStd);

    glUniform1f(glGetUniformLocation(shaderHandle, "camera.pitch"), mCamera.pitch);
    glUniform1f(glGetUniformLocation(shaderHandle, "camera.yaw"), mCamera.yaw);
    glUniform1f(glGetUniformLocation(shaderHandle, "camera.fov"), mCamera.fov);
    glUniform1i(glGetUniformLocation(shaderHandle, "camera.projection"), mCamera.projection);
    glUniform1i(glGetUniformLocation(shaderHandle, "camera.hideSubHorizon"), mCamera.hideSubHorizon ? 1 : 0);

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
    mSimulationTexture = std::make_unique<OpenGL::Texture>(mOutputWidth, mOutputHeight, 0, OpenGL::TextureType::Color);
    mSpinlockTexture = std::make_unique<OpenGL::Texture>(mOutputWidth, mOutputHeight, 1, OpenGL::TextureType::Monochrome);
}

void SimulationEngine::ResizeOutputTextureCallback(const unsigned int width, const unsigned int height)
{
    mOutputWidth = width;
    mOutputHeight = height;

    mSimulationTexture.reset();
    mSpinlockTexture.reset();

    InitializeTextures();
    Clear();
}

} // namespace HaloSim
