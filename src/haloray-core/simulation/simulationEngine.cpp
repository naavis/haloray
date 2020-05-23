#include "simulationEngine.h"
#include <memory>
#include <random>
#include <limits>
#include <stdexcept>
#include <QOpenGLShaderProgram>
#include "../opengl/texture.h"
#include "camera.h"
#include "lightSource.h"
#include "crystalPopulation.h"

namespace HaloSim
{

SimulationEngine::SimulationEngine(
    std::shared_ptr<CrystalPopulationRepository> crystalRepository)
    : mOutputWidth(800),
      mOutputHeight(600),
      mMersenneTwister(std::mt19937(std::random_device()())),
      mUniformDistribution(std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max())),
      mRunning(false),
      mRaysPerStep(500000),
      mIteration(0),
      mInitialized(false),
      mCamera(Camera::createDefaultCamera()),
      mLight(LightSource::createDefaultLightSource()),
      mCameraLockedToLightSource(false),
      mMultipleScatteringProbability(0.0),
      mCrystalRepository(crystalRepository)
{
}

bool SimulationEngine::isRunning() const
{
    return mRunning;
}

Camera SimulationEngine::getCamera() const
{
    return mCamera;
}

void SimulationEngine::setCamera(const Camera camera)
{
    clear();
    mCamera = camera;
    if (mCameraLockedToLightSource)
    {
        pointCameraToLightSource();
    }
}

LightSource SimulationEngine::getLightSource() const
{
    return mLight;
}

void SimulationEngine::setLightSource(const LightSource light)
{
    clear();
    mLight = light;
    if (mCameraLockedToLightSource)
    {
        pointCameraToLightSource();
    }
}

unsigned int SimulationEngine::getOutputTextureHandle() const
{
    return mSimulationTexture->getHandle();
}

unsigned int SimulationEngine::getIteration() const
{
    return mIteration;
}

void SimulationEngine::start()
{
    if (isRunning())
        return;
    clear();
    mRunning = true;
    mIteration = 0;
}

void SimulationEngine::stop()
{
    mRunning = false;
}

void SimulationEngine::step()
{
    ++mIteration;

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(mSimulationTexture->getTextureUnit(), mSimulationTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(mSpinlockTexture->getHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(mSpinlockTexture->getTextureUnit(), mSpinlockTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    mSimulationShader->bind();

    for (auto i = 0u; i < mCrystalRepository->getCount(); ++i)
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        unsigned int seed = mUniformDistribution(mMersenneTwister);

        auto crystals = mCrystalRepository->get(i);
        auto probability = mCrystalRepository->getProbability(i);
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

        mSimulationShader->setUniformValue("multipleScatter", mMultipleScatteringProbability);

        glDispatchCompute(numRays, 1, 1);
    }
}

void SimulationEngine::clear()
{
    if (!mInitialized)
        return;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glClearTexImage(mSimulationTexture->getHandle(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(mSimulationTexture->getTextureUnit(), mSimulationTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(mSpinlockTexture->getHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(mSpinlockTexture->getTextureUnit(), mSpinlockTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    mIteration = 0;
}

unsigned int SimulationEngine::getRaysPerStep() const
{
    return mRaysPerStep;
}

void SimulationEngine::setRaysPerStep(unsigned int rays)
{
    clear();
    mRaysPerStep = rays;
}

void SimulationEngine::initialize()
{
    if (mInitialized)
        return;
    initializeOpenGLFunctions();
    initializeShader();
    initializeTextures();
    mInitialized = true;
}

void SimulationEngine::initializeShader()
{
    mSimulationShader = std::make_unique<QOpenGLShaderProgram>();
#ifdef _WIN32
    mSimulationShader->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Compute, ":/shaders/raytrace.glsl");
#else
    mSimulationShader->addShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Compute, ":/shaders/raytrace.glsl");
#endif
    if (mSimulationShader->link() == false)
    {
        throw std::runtime_error(mSimulationShader->log().toUtf8());
    }
}

void SimulationEngine::initializeTextures()
{
    mSimulationTexture = std::make_unique<OpenGL::Texture>(mOutputWidth, mOutputHeight, 0, OpenGL::TextureType::Color);
    mSpinlockTexture = std::make_unique<OpenGL::Texture>(mOutputWidth, mOutputHeight, 1, OpenGL::TextureType::Monochrome);
}

void SimulationEngine::resizeOutputTextureCallback(const unsigned int width, const unsigned int height)
{
    mOutputWidth = width;
    mOutputHeight = height;

    mSimulationTexture.reset();
    mSpinlockTexture.reset();

    initializeTextures();
    clear();
}

void SimulationEngine::lockCameraToLightSource(bool locked)
{
    mCameraLockedToLightSource = locked;
    pointCameraToLightSource();
}

void SimulationEngine::pointCameraToLightSource()
{
    clear();
    mCamera.yaw = 0.0f;
    mCamera.pitch = mLight.altitude;
}

void SimulationEngine::setMultipleScatteringProbability(double probability)
{
    clear();
    mMultipleScatteringProbability = static_cast<float>(std::min(std::max(probability, 0.0), 1.0));
}

double SimulationEngine::getMultipleScatteringProbability() const
{
    return static_cast<double>(mMultipleScatteringProbability);
}

} // namespace HaloSim
