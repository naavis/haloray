#pragma once
#include <random>
#include <memory>
#include <QObject>
#include <QOpenGLFunctions_4_4_Core>
#include <QOpenGLShaderProgram>
#include "../opengl/texture.h"
#include "camera.h"
#include "atmosphere.h"
#include "lightSource.h"
#include "crystalPopulation.h"
#include "crystalPopulationRepository.h"

namespace HaloRay
{

class SimulationEngine : public QObject, protected QOpenGLFunctions_4_4_Core
{
    Q_OBJECT
public:
    SimulationEngine(std::shared_ptr<CrystalPopulationRepository> crystalRepository, QObject *parent = nullptr);
    void initialize();
    void start();
    void step();
    void stop();
    bool isRunning() const;

    void clear();

    unsigned int getIteration() const;

    unsigned int getRaysPerStep() const;
    void setRaysPerStep(unsigned int rays);

    Camera getCamera() const;
    void setCamera(const Camera);

    LightSource getLightSource() const;
    void setLightSource(const LightSource);

    Atmosphere getAtmosphere() const;
    void setAtmosphere(Atmosphere);

    void lockCameraToLightSource(bool locked);

    void setMultipleScatteringProbability(double);
    double getMultipleScatteringProbability() const;

    unsigned int getOutputTextureHandle() const;
    unsigned int getBackgroundTextureHandle() const;

    void resizeOutputTextureCallback(const unsigned int width, const unsigned int height);

signals:
    void raysPerStepChanged(unsigned int);
    void cameraChanged(Camera);
    void lightSourceChanged(LightSource);
    void atmosphereChanged(Atmosphere);
    void lockCameraToLightSourceChanged(bool);
    void multipleScatteringProbabilityChanged(double);

private:
    void initializeShaders();
    void initializeTextures();
    void pointCameraToLightSource();

    unsigned int m_outputWidth;
    unsigned int m_outputHeight;
    std::mt19937 m_mersenneTwister;
    std::uniform_int_distribution<unsigned int> m_uniformDistribution;
    QOpenGLShaderProgram *m_simulationShader;
    QOpenGLShaderProgram *m_skyShader;
    std::unique_ptr<OpenGL::Texture> m_simulationTexture;
    std::unique_ptr<OpenGL::Texture> m_backgroundTexture;

    Camera m_camera;
    LightSource m_light;

    bool m_running;
    bool m_initialized;
    unsigned int m_raysPerStep;
    unsigned int m_iteration;
    bool m_cameraLockedToLightSource;
    float m_multipleScatteringProbability;
    std::shared_ptr<CrystalPopulationRepository> m_crystalRepository;
    float m_sunSpectrumCache[31];
    Atmosphere m_atmosphere;
};

}
