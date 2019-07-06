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
    void Initialize();
    void Start();
    void Step();
    void Stop();
    bool IsRunning() const;

    void Clear();

    unsigned int GetIteration() const;

    unsigned int GetRaysPerStep() const;
    void SetRaysPerStep(unsigned int rays);

    Camera GetCamera() const;
    void SetCamera(const Camera);

    LightSource GetLightSource() const;
    void SetLightSource(const LightSource);

    void LockCameraToLightSource(bool locked);

    const unsigned int GetOutputTextureHandle() const;

    void ResizeOutputTextureCallback(const unsigned int width, const unsigned int height);

private:
    void InitializeShader();
    void InitializeTextures();
    void PointCameraToLightSource();

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
