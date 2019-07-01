#include "simulationEngine.h"
#include <memory>
#include <random>
#include <limits>
#include <QOpenGLShaderProgram>
#include "../opengl/texture.h"
#include "defaults.h"
#include "camera.h"
#include "lightSource.h"
#include "crystalPopulation.h"
#include "shaders/raytrace.glsl"

namespace HaloSim
{

SimulationEngine::SimulationEngine(
    unsigned int outputWidth,
    unsigned int outputHeight)
    : mOutputWidth(outputWidth),
      mOutputHeight(outputHeight),
      mMersenneTwister(std::mt19937(std::random_device()())),
      mUniformDistribution(std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max())),
      mRunning(false),
      mRaysPerStep(500000),
      mIteration(0),
      mInitialized(false),
      mCamera(DefaultCamera()),
      mLight(DefaultLightSource()),
      mCrystals(DefaultCrystalPopulation())
{
}

bool SimulationEngine::IsRunning() const
{
    return mRunning;
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

unsigned int SimulationEngine::GetIteration() const
{
    return mIteration;
}

void SimulationEngine::Start()
{
    if (IsRunning())
        return;
    Clear();
    mRunning = true;
    mIteration = 0;
}

void SimulationEngine::Stop()
{
    mRunning = false;
}

void SimulationEngine::Step()
{
    ++mIteration;

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(mSimulationTexture->GetTextureUnit(), mSimulationTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(mSpinlockTexture->GetHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(mSpinlockTexture->GetTextureUnit(), mSpinlockTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    mSimulationShader->bind();

    unsigned int seed = mUniformDistribution(mMersenneTwister);

    /* The following line needs to use glUniform1ui instead of the
       setUniformValue method because of a bug in Qt:
       https://bugreports.qt.io/browse/QTBUG-45507
    */
    glUniform1ui(glGetUniformLocation(mSimulationShader->programId(), "rngSeed"), seed);
    mSimulationShader->setUniformValue("sun.altitude", mLight.altitude);
    mSimulationShader->setUniformValue("sun.diameter", mLight.diameter);

    mSimulationShader->setUniformValue("crystalProperties.caRatioAverage", mCrystals.caRatioAverage);
    mSimulationShader->setUniformValue("crystalProperties.caRatioStd", mCrystals.caRatioStd);

    mSimulationShader->setUniformValue("crystalProperties.tiltDistribution", mCrystals.tiltDistribution);
    mSimulationShader->setUniformValue("crystalProperties.tiltAverage", mCrystals.tiltAverage);
    mSimulationShader->setUniformValue("crystalProperties.tiltStd", mCrystals.tiltStd);

    mSimulationShader->setUniformValue("crystalProperties.rotationDistribution", mCrystals.rotationDistribution);
    mSimulationShader->setUniformValue("crystalProperties.rotationAverage", mCrystals.rotationAverage);
    mSimulationShader->setUniformValue("crystalProperties.rotationStd", mCrystals.rotationStd);

    mSimulationShader->setUniformValue("camera.pitch", mCamera.pitch);
    mSimulationShader->setUniformValue("camera.yaw", mCamera.yaw);
    mSimulationShader->setUniformValue("camera.fov", mCamera.fov);
    mSimulationShader->setUniformValue("camera.projection", mCamera.projection);
    mSimulationShader->setUniformValue("camera.hideSubHorizon", mCamera.hideSubHorizon ? 1 : 0);

    glDispatchCompute(mRaysPerStep, 1, 1);
}

void SimulationEngine::Clear()
{
    if (!mInitialized)
        return;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glClearTexImage(mSimulationTexture->GetHandle(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(mSimulationTexture->GetTextureUnit(), mSimulationTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(mSpinlockTexture->GetHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(mSpinlockTexture->GetTextureUnit(), mSpinlockTexture->GetHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    mIteration = 0;
}

unsigned int SimulationEngine::GetRaysPerStep() const
{
    return mRaysPerStep;
}

void SimulationEngine::SetRaysPerStep(unsigned int rays)
{
    Clear();
    mRaysPerStep = rays;
}

void SimulationEngine::Initialize()
{
    if (mInitialized)
        return;
    initializeOpenGLFunctions();
    InitializeShader();
    InitializeTextures();
    mInitialized = true;
}

void SimulationEngine::InitializeShader()
{
    mSimulationShader = std::make_unique<QOpenGLShaderProgram>();
    mSimulationShader->addCacheableShaderFromSourceCode(QOpenGLShader::ShaderTypeBit::Compute, computeShaderSource.c_str());
    if (mSimulationShader->link() == false)
    {
        throw std::runtime_error(mSimulationShader->log().toUtf8());
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
