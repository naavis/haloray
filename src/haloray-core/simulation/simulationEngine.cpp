#include "simulationEngine.h"
#include <memory>
#include <random>
#include <limits>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include "../opengl/texture.h"
#include "trigonometryUtilities.h"
#include "camera.h"
#include "lightSource.h"
#include "crystalPopulation.h"
#include "hosekWilkie/ArHosekSkyModel.h"
#include "skyModel.h"

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
      m_crystalRepository(crystalRepository),
      m_atmosphere(Atmosphere::createDefaultAtmosphere())
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

Atmosphere SimulationEngine::getAtmosphere() const
{
    return m_atmosphere;
}

void SimulationEngine::setAtmosphere(Atmosphere atmosphere)
{
    if (m_atmosphere == atmosphere) return;

    clear();
    m_atmosphere = atmosphere;
    emit atmosphereChanged(m_atmosphere);
}

unsigned int SimulationEngine::getOutputTextureHandle() const
{
    return m_simulationTexture->getHandle();
}

unsigned int SimulationEngine::getBackgroundTextureHandle() const
{
    return m_backgroundTexture->getHandle();
}

unsigned int SimulationEngine::getGuideTextureHandle() const
{
    return m_guideTexture->getHandle();
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

    if (m_iteration == 1)
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(m_guideTexture->getTextureUnit(), m_guideTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
        m_guideShader->bind();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        m_guideShader->setUniformValue("sun.altitude", degToRad(m_light.altitude));
        m_guideShader->setUniformValue("sun.diameter", degToRad(m_light.diameter));
        m_guideShader->setUniformValue("camera.pitch", degToRad(m_camera.pitch));
        m_guideShader->setUniformValue("camera.yaw", degToRad(m_camera.yaw));
        m_guideShader->setUniformValue("camera.focalLength", m_camera.getFocalLength());
        m_guideShader->setUniformValue("camera.projection", m_camera.projection);
        m_guideShader->setUniformValue("camera.hideSubHorizon", m_camera.hideSubHorizon ? 1 : 0);

        glDispatchCompute(m_outputWidth, m_outputHeight, 1);
    }

    if (m_atmosphere.enabled && m_iteration == 1)
    {
        auto skyState = SkyModel::Create(degToRad(m_light.altitude), m_atmosphere.turbidity, m_atmosphere.groundAlbedo, degToRad(m_light.diameter / 2.0));

        for (auto i = 0u; i < 31; ++i) {
            m_sunSpectrumCache[i] = skyState.sunSpectrum[i];
        }

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindImageTexture(m_backgroundTexture->getTextureUnit(), m_backgroundTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        m_skyShader->bind();
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        m_skyShader->setUniformValue("sun.altitude", degToRad(m_light.altitude));
        m_skyShader->setUniformValue("sun.diameter", degToRad(m_light.diameter));
        m_skyShader->setUniformValue("camera.pitch", degToRad(m_camera.pitch));
        m_skyShader->setUniformValue("camera.yaw", degToRad(m_camera.yaw));
        m_skyShader->setUniformValue("camera.focalLength", m_camera.getFocalLength());
        m_skyShader->setUniformValue("camera.projection", m_camera.projection);
        m_skyShader->setUniformValue("camera.hideSubHorizon", m_camera.hideSubHorizon ? 1 : 0);

        for (auto channel = 0u; channel < 3; ++channel)
        {
            auto configLocation = m_skyShader->uniformLocation(QString("skyModelState.configs[%1]").arg(channel));
            m_skyShader->setUniformValueArray(configLocation, skyState.configs[channel], 9, 1);
        }
        m_skyShader->setUniformValueArray("skyModelState.radiances", skyState.radiances, 3, 1);
        m_skyShader->setUniformValue("skyModelState.turbidity", skyState.turbidity);
        m_skyShader->setUniformValue("skyModelState.solarRadius", degToRad(m_light.diameter / 2.0f));
        m_skyShader->setUniformValue("skyModelState.elevation", degToRad(m_light.altitude));
        m_skyShader->setUniformValue("skyModelState.sunTopCIEXYZ", skyState.sunTopCIEXYZ[0], skyState.sunTopCIEXYZ[1], skyState.sunTopCIEXYZ[2]);
        m_skyShader->setUniformValue("skyModelState.sunBottomCIEXYZ", skyState.sunBottomCIEXYZ[0], skyState.sunBottomCIEXYZ[1], skyState.sunBottomCIEXYZ[2]);
        m_skyShader->setUniformValue("skyModelState.limbDarkeningScaler", skyState.limbDarkeningScaler[0], skyState.limbDarkeningScaler[1], skyState.limbDarkeningScaler[2]);

        glDispatchCompute(m_outputWidth, m_outputHeight, 1);
    }

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(m_simulationTexture->getTextureUnit(), m_simulationTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

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
        m_simulationShader->setUniformValue("sun.altitude", degToRad(m_light.altitude));
        m_simulationShader->setUniformValue("sun.diameter", degToRad(m_light.diameter));
        m_simulationShader->setUniformValueArray("sun.spectrum", m_sunSpectrumCache, 31, 1);

        m_simulationShader->setUniformValue("crystalProperties.caRatioAverage", crystals.caRatioAverage);
        m_simulationShader->setUniformValue("crystalProperties.caRatioStd", crystals.caRatioStd);

        m_simulationShader->setUniformValue("crystalProperties.tiltDistribution", crystals.tiltDistribution);
        m_simulationShader->setUniformValue("crystalProperties.tiltAverage", degToRad(crystals.tiltAverage));
        m_simulationShader->setUniformValue("crystalProperties.tiltStd", degToRad(crystals.tiltStd));

        m_simulationShader->setUniformValue("crystalProperties.rotationDistribution", crystals.rotationDistribution);
        m_simulationShader->setUniformValue("crystalProperties.rotationAverage", degToRad(crystals.rotationAverage));
        m_simulationShader->setUniformValue("crystalProperties.rotationStd", degToRad(crystals.rotationStd));

        m_simulationShader->setUniformValue("crystalProperties.upperApexAngle", degToRad(crystals.upperApexAngle));
        m_simulationShader->setUniformValue("crystalProperties.upperApexHeightAverage", crystals.upperApexHeightAverage);
        m_simulationShader->setUniformValue("crystalProperties.upperApexHeightStd", crystals.upperApexHeightStd);

        m_simulationShader->setUniformValue("crystalProperties.lowerApexAngle", degToRad(crystals.lowerApexAngle));
        m_simulationShader->setUniformValue("crystalProperties.lowerApexHeightAverage", crystals.lowerApexHeightAverage);
        m_simulationShader->setUniformValue("crystalProperties.lowerApexHeightStd", crystals.lowerApexHeightStd);
        m_simulationShader->setUniformValueArray("crystalProperties.prismFaceDistances", crystals.prismFaceDistances, 6, 1);

        m_simulationShader->setUniformValue("camera.pitch", degToRad(m_camera.pitch));
        m_simulationShader->setUniformValue("camera.yaw", degToRad(m_camera.yaw));
        m_simulationShader->setUniformValue("camera.focalLength", m_camera.getFocalLength());
        m_simulationShader->setUniformValue("camera.projection", m_camera.projection);
        m_simulationShader->setUniformValue("camera.hideSubHorizon", m_camera.hideSubHorizon ? 1 : 0);

        m_simulationShader->setUniformValue("multipleScatter", m_multipleScatteringProbability);
        m_simulationShader->setUniformValue("atmosphereEnabled", m_atmosphere.enabled ? 1 : 0);

        glDispatchCompute(numRays / 64.0, 1, 1);
    }
}

void SimulationEngine::clear()
{
    if (!m_initialized)
        return;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glClearTexImage(m_simulationTexture->getHandle(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(m_simulationTexture->getTextureUnit(), m_simulationTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glClearTexImage(m_backgroundTexture->getHandle(), 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(m_backgroundTexture->getTextureUnit(), m_backgroundTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glClearTexImage(m_guideTexture->getHandle(), 0, GL_R, GL_FLOAT, NULL);
    glBindImageTexture(m_guideTexture->getTextureUnit(), m_guideTexture->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

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
    qInfo("Initializing raytracing shader");
    m_simulationShader = new QOpenGLShaderProgram(this);
    bool raytraceShaderReadSucceeded = m_simulationShader->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Compute, ":/shaders/raytrace.glsl");
    if (raytraceShaderReadSucceeded == false)
    {
        qWarning("Reading raytracing shader failed");
        throw std::runtime_error(m_simulationShader->log().toUtf8());
    }
    qInfo("Raytracing shader successfully initialized");

    if (m_simulationShader->link() == false)
    {
        qWarning("Compiling and linking raytracing shader failed");
        throw std::runtime_error(m_simulationShader->log().toUtf8());
    }
    qInfo("Raytracing shader program compilation and linking successful");

    qInfo("Initializing sky shader");
    m_skyShader = new QOpenGLShaderProgram(this);
    bool skyShaderReadSucceeded = m_skyShader->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Compute, ":/shaders/sky.glsl");
    if (skyShaderReadSucceeded == false)
    {
        qWarning("Reading sky shader failed");
        throw std::runtime_error(m_skyShader->log().toUtf8());
    }
    qInfo("Sky shader successfully initialized");

    if (m_skyShader->link() == false)
    {
        qWarning("Compiling and linking sky shader failed");
        throw std::runtime_error(m_skyShader->log().toUtf8());
    }
    qInfo("Sky shader program compilation and linking successful");

    qInfo("Initializing guide marking shader");
    m_guideShader = new QOpenGLShaderProgram(this);
    bool guideShaderReadSucceeded = m_guideShader->addCacheableShaderFromSourceFile(QOpenGLShader::ShaderTypeBit::Compute, ":/shaders/guide.glsl");
    if (guideShaderReadSucceeded == false)
    {
        qWarning("Reading guide marking shader failed");
        throw std::runtime_error(m_guideShader->log().toUtf8());
    }
    qInfo("Guide marking shader successfully initialized");

    if (m_guideShader->link() == false)
    {
        qWarning("Compiling and linking guide marking shader failed");
        throw std::runtime_error(m_guideShader->log().toUtf8());
    }
    qInfo("Guide marking shader program compilation and linking successful");
}

void SimulationEngine::initializeTextures()
{
    m_simulationTexture = std::make_unique<OpenGL::Texture>(m_outputWidth, m_outputHeight, 0, OpenGL::TextureType::Color);
    m_backgroundTexture = std::make_unique<OpenGL::Texture>(m_outputWidth, m_outputHeight, 1, OpenGL::TextureType::Color);
    m_guideTexture = std::make_unique<OpenGL::Texture>(m_outputWidth, m_outputHeight, 2, OpenGL::TextureType::Monochrome);
}

void SimulationEngine::resizeOutputTextureCallback(const unsigned int width, const unsigned int height)
{
    m_outputWidth = width;
    m_outputHeight = height;

    m_simulationTexture.reset();
    m_backgroundTexture.reset();
    m_guideTexture.reset();

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
