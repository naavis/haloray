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
    unsigned int outputHeight,
    std::shared_ptr<CrystalPopulationRepository> crystalRepository)
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
      mCameraLockedToLightSource(false),
      mCrystalRepository(crystalRepository)
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

void SimulationEngine::SetCamera(const Camera camera)
{
    Clear();
    mCamera = camera;
    if (mCameraLockedToLightSource)
    {
        PointCameraToLightSource();
    }
}

LightSource SimulationEngine::GetLightSource() const
{
    return mLight;
}

void SimulationEngine::SetLightSource(const LightSource light)
{
    Clear();
    mLight = light;
    if (mCameraLockedToLightSource)
    {
        PointCameraToLightSource();
    }
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

    for (auto i = 0u; i < mCrystalRepository->GetCount(); ++i)
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        unsigned int seed = mUniformDistribution(mMersenneTwister);

        auto crystals = mCrystalRepository->Get(i);
        auto probability = mCrystalRepository->GetProbability(i);
        auto numRays = static_cast<unsigned int>(mRaysPerStep * probability);

        /*
        The following line needs to use glUniform1ui instead of the
        setUniformValue method because of a bug in Qt:
        https://bugreports.qt.io/browse/QTBUG-45507
        */
        glUniform1ui(glGetUniformLocation(mSimulationShader->programId(), "rngSeed"), seed);
        mSimulationShader->setUniformValue("sun.altitude", mLight.altitude);
        mSimulationShader->setUniformValue("sun.diameter", mLight.diameter);

        mSimulationShader->setUniformValue("crystalProperties.caRatioAverage", crystals.caRatioAverage);
        mSimulationShader->setUniformValue("crystalProperties.caRatioStd", crystals.caRatioStd);

        mSimulationShader->setUniformValue("crystalProperties.tiltDistribution", crystals.tiltDistribution);
        mSimulationShader->setUniformValue("crystalProperties.tiltAverage", crystals.tiltAverage);
        mSimulationShader->setUniformValue("crystalProperties.tiltStd", crystals.tiltStd);

        mSimulationShader->setUniformValue("crystalProperties.rotationDistribution", crystals.rotationDistribution);
        mSimulationShader->setUniformValue("crystalProperties.rotationAverage", crystals.rotationAverage);
        mSimulationShader->setUniformValue("crystalProperties.rotationStd", crystals.rotationStd);

        mSimulationShader->setUniformValue("camera.pitch", mCamera.pitch);
        mSimulationShader->setUniformValue("camera.yaw", mCamera.yaw);
        mSimulationShader->setUniformValue("camera.fov", mCamera.fov);
        mSimulationShader->setUniformValue("camera.projection", mCamera.projection);
        mSimulationShader->setUniformValue("camera.hideSubHorizon", mCamera.hideSubHorizon ? 1 : 0);

        glDispatchCompute(numRays, 1, 1);
    }
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

void SimulationEngine::LockCameraToLightSource(bool locked)
{
    mCameraLockedToLightSource = locked;
    PointCameraToLightSource();
}

void SimulationEngine::PointCameraToLightSource()
{
    Clear();
    mCamera.yaw = 0.0f;
    mCamera.pitch = mLight.altitude;
}

} // namespace HaloSim
