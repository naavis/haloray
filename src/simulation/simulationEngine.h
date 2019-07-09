#pragma once
#include <random>
#include <memory>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_4_4_Core>
#include "../opengl/texture.h"
#include "camera.h"
#include "lightSource.h"
#include "crystalPopulation.h"
#include "crystalPopulationRepository.h"

namespace HaloSim
{

class SimulationEngine : protected QOpenGLFunctions_4_4_Core
{
public:
    SimulationEngine(unsigned int outputWidth, unsigned int outputHeight, std::shared_ptr<CrystalPopulationRepository> crystalRepository);
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

    void lockCameraToLightSource(bool locked);

    const unsigned int getOutputTextureHandle() const;

    void resizeOutputTextureCallback(const unsigned int width, const unsigned int height);

private:
    void initializeShader();
    void initializeTextures();
    void pointCameraToLightSource();

    unsigned int mOutputWidth;
    unsigned int mOutputHeight;
    std::mt19937 mMersenneTwister;
    std::uniform_int_distribution<unsigned int> mUniformDistribution;
    std::unique_ptr<QOpenGLShaderProgram> mSimulationShader;
    std::unique_ptr<OpenGL::Texture> mSimulationTexture;
    std::unique_ptr<OpenGL::Texture> mSpinlockTexture;

    Camera mCamera;
    LightSource mLight;

    bool mRunning;
    bool mInitialized;
    unsigned int mRaysPerStep;
    unsigned int mIteration;
    bool mCameraLockedToLightSource;
    std::shared_ptr<CrystalPopulationRepository> mCrystalRepository;
};

} // namespace HaloSim
