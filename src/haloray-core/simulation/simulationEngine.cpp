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

namespace HaloRay
{

SimulationEngine::SimulationEngine(
    std::shared_ptr<CrystalPopulationRepository> crystalRepository,
    QObject *parent)
    : QObject(parent),
      m_outputWidth(800),
      m_outputHeight(600),
      m_mersenneTwister(std::mt19937(std::random_device()())),
      m_uniformDistribution(std::uniform_int_distribution<unsigned int>(0, std::numeric_limits<unsigned int>::max())),
      m_camera(Camera::createDefaultCamera()),
      m_light(LightSource::createDefaultLightSource()),
      m_running(false),
      m_initialized(false),
      m_raysPerStep(500000),
      m_iteration(0),
      m_cameraLockedToLightSource(false),
      m_multipleScatteringProbability(0.0),
      m_crystalRepository(crystalRepository)
{
    initialize();
}

bool SimulationEngine::isRunning() const
{
    return m_running;
}

Camera SimulationEngine::getCamera() const
{
    return m_camera;
}

void SimulationEngine::setCamera(const Camera camera)
{
    if (m_camera == camera) return;

    clear();
    m_camera = camera;
    if (m_cameraLockedToLightSource)
    {
        pointCameraToLightSource();
    }
    emit cameraChanged(m_camera);
}

LightSource SimulationEngine::getLightSource() const
{
    return m_light;
}

void SimulationEngine::setLightSource(const LightSource light)
{
    if (m_light == light) return;

    clear();
    m_light = light;
    if (m_cameraLockedToLightSource)
    {
        pointCameraToLightSource();
    }

    emit lightSourceChanged(m_light);
}

unsigned int SimulationEngine::getOutputTextureHandle() const
{
    return m_simulationTexture->getHandle();
}

unsigned int SimulationEngine::getIteration() const
{
    return m_iteration;
}

void SimulationEngine::start()
{
    if (isRunning())
        return;
    clear();
    m_running = true;
    m_iteration = 0;
}

void SimulationEngine::stop()
{
    m_running = false;
}

void SimulationEngine::step()
{
    ++m_iteration;

    if (m_iteration == 1) {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(m_simulationTexture->getTextureUnit(), m_simulationTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        m_skyShader->bind();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        m_skyShader->setUniformValue("sun.altitude", m_light.altitude);
        m_skyShader->setUniformValue("sun.diameter", m_light.diameter);
        m_skyShader->setUniformValue("camera.pitch", m_camera.pitch);
        m_skyShader->setUniformValue("camera.yaw", m_camera.yaw);
        m_skyShader->setUniformValue("camera.fov", m_camera.fov);
        m_skyShader->setUniformValue("camera.projection", m_camera.projection);
        m_skyShader->setUniformValue("camera.hideSubHorizon", m_camera.hideSubHorizon ? 1 : 0);
        glDispatchCompute(m_outputWidth, m_outputHeight, 1);
    }

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(m_simulationTexture->getTextureUnit(), m_simulationTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(m_spinlockTexture->getHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(m_spinlockTexture->getTextureUnit(), m_spinlockTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    m_simulationShader->bind();

    for (auto i = 0u; i < m_crystalRepository->getCount(); ++i)
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        unsigned int seed = m_uniformDistribution(m_mersenneTwister);

        auto crystals = m_crystalRepository->get(i);
        auto probability = m_crystalRepository->getProbability(i);
        auto numRays = static_cast<unsigned int>(m_raysPerStep * probability);

        /*
        The following line needs to use glUniform1ui instead of the
        setUniformValue method because of a bug in Qt:
        https://bugreports.qt.io/browse/QTBUG-45507
        */
        glUniform1ui(glGetUniformLocation(m_simulationShader->programId(), "rngSeed"), seed);
        m_simulationShader->setUniformValue("sun.altitude", m_light.altitude);
        m_simulationShader->setUniformValue("sun.diameter", m_light.diameter);

        m_simulationShader->setUniformValue("crystalProperties.caRatioAverage", crystals.caRatioAverage);
        m_simulationShader->setUniformValue("crystalProperties.caRatioStd", crystals.caRatioStd);

        m_simulationShader->setUniformValue("crystalProperties.tiltDistribution", crystals.tiltDistribution);
        m_simulationShader->setUniformValue("crystalProperties.tiltAverage", crystals.tiltAverage);
        m_simulationShader->setUniformValue("crystalProperties.tiltStd", crystals.tiltStd);

        m_simulationShader->setUniformValue("crystalProperties.rotationDistribution", crystals.rotationDistribution);
        m_simulationShader->setUniformValue("crystalProperties.rotationAverage", crystals.rotationAverage);
        m_simulationShader->setUniformValue("crystalProperties.rotationStd", crystals.rotationStd);

        m_simulationShader->setUniformValue("camera.pitch", m_camera.pitch);
        m_simulationShader->setUniformValue("camera.yaw", m_camera.yaw);
        m_simulationShader->setUniformValue("camera.fov", m_camera.fov);
        m_simulationShader->setUniformValue("camera.projection", m_camera.projection);
        m_simulationShader->setUniformValue("camera.hideSubHorizon", m_camera.hideSubHorizon ? 1 : 0);

        m_simulationShader->setUniformValue("multipleScatter", m_multipleScatteringProbability);

        glDispatchCompute(numRays, 1, 1);
    }
}

void SimulationEngine::clear()
{
    if (!m_initialized)
        return;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glClearTexImage(m_simulationTexture->getHandle(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(m_simulationTexture->getTextureUnit(), m_simulationTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glClearTexImage(m_spinlockTexture->getHandle(), 0, GL_RED, GL_UNSIGNED_INT, NULL);
    glBindImageTexture(m_spinlockTexture->getTextureUnit(), m_spinlockTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    m_iteration = 0;
}

unsigned int SimulationEngine::getRaysPerStep() const
{
    return m_raysPerStep;
}

void SimulationEngine::setRaysPerStep(unsigned int rays)
{
    if (m_raysPerStep == rays) return;

    clear();
    m_raysPerStep = rays;

    emit raysPerStepChanged(m_raysPerStep);
}

void SimulationEngine::initialize()
{
    if (m_initialized)
        return;
    initializeOpenGLFunctions();
    initializeShaders();
    initializeTextures();
    m_initialized = true;
}

void SimulationEngine::initializeShaders()
{
    m_simulationShader = std::make_unique<QOpenGLShaderProgram>();
    m_simulationShader->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Compute, ":/shaders/raytrace.glsl");
    if (m_simulationShader->link() == false)
    {
        throw std::runtime_error(m_simulationShader->log().toUtf8());
    }

    m_skyShader = new QOpenGLShaderProgram(this);
    m_skyShader->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Compute, ":/shaders/sky.glsl");
    if (m_skyShader->link() == false)
    {
        throw std::runtime_error(m_skyShader->log().toUtf8());
    }
}

void SimulationEngine::initializeTextures()
{
    m_simulationTexture = std::make_unique<OpenGL::Texture>(m_outputWidth, m_outputHeight, 0, OpenGL::TextureType::Color);
    m_spinlockTexture = std::make_unique<OpenGL::Texture>(m_outputWidth, m_outputHeight, 1, OpenGL::TextureType::Monochrome);
}

void SimulationEngine::resizeOutputTextureCallback(const unsigned int width, const unsigned int height)
{
    m_outputWidth = width;
    m_outputHeight = height;

    m_simulationTexture.reset();
    m_spinlockTexture.reset();

    initializeTextures();
    clear();
}

void SimulationEngine::lockCameraToLightSource(bool locked)
{
    if (m_cameraLockedToLightSource == locked) return;

    m_cameraLockedToLightSource = locked;
    pointCameraToLightSource();

    emit cameraChanged(m_camera);
    emit lockCameraToLightSourceChanged(m_cameraLockedToLightSource);
}

void SimulationEngine::pointCameraToLightSource()
{
    if (m_camera.yaw == 0.0f && m_camera.pitch == m_light.altitude) return;
    clear();
    m_camera.yaw = 0.0f;
    m_camera.pitch = m_light.altitude;
}

void SimulationEngine::setMultipleScatteringProbability(double probability)
{
    if (m_multipleScatteringProbability == probability) return;

    clear();
    m_multipleScatteringProbability = static_cast<float>(std::min(std::max(probability, 0.0), 1.0));

    emit multipleScatteringProbabilityChanged(m_multipleScatteringProbability);
}

double SimulationEngine::getMultipleScatteringProbability() const
{
    return static_cast<double>(m_multipleScatteringProbability);
}

}
