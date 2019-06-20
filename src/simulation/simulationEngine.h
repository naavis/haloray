#pragma once
#include <random>
#include <memory>
#include <QOpenGLFunctions_4_4_Core>
#include "../opengl/shader.h"
#include "../opengl/program.h"
#include "../opengl/texture.h"

namespace HaloSim
{

struct CrystalPopulation
{
    float caRatioAverage;
    float caRatioStd;

    int tiltDistribution;
    float tiltAverage;
    float tiltStd;

    int rotationDistribution;
    float rotationAverage;
    float rotationStd;

    bool operator==(const struct CrystalPopulation &other) const
    {
        return caRatioAverage == other.caRatioAverage &&
               caRatioStd == other.caRatioStd &&
               tiltDistribution == other.tiltDistribution &&
               tiltAverage == other.tiltAverage &&
               tiltStd == other.tiltStd &&
               rotationDistribution == other.rotationDistribution &&
               rotationAverage == other.rotationAverage &&
               rotationStd == other.rotationStd;
    }

    bool operator!=(const struct CrystalPopulation &other) const
    {
        return !operator==(other);
    }
};

struct LightSource
{
    float altitude;
    float diameter;

    bool operator==(const struct LightSource &other) const
    {
        return altitude == other.altitude &&
               diameter == other.diameter;
    }

    bool operator!=(const struct LightSource &other) const
    {
        return !operator==(other);
    }
};

enum Projection
{
    Stereographic = 0,
    Rectilinear,
    Equidistant,
    EqualArea,
    Orthographic
};

struct Camera
{
    float pitch;
    float yaw;
    float fov;
    Projection projection;
    bool hideSubHorizon;

    bool operator==(const struct Camera &other) const
    {
        return pitch == other.pitch &&
               yaw == other.yaw &&
               fov == other.fov &&
               projection == other.projection &&
               hideSubHorizon == other.hideSubHorizon;
    }

    bool operator!=(const struct Camera &other) const
    {
        return !operator==(other);
    }
};

class SimulationEngine : protected QOpenGLFunctions_4_4_Core
{
public:
    SimulationEngine(unsigned int outputWidth, unsigned int outputHeight);
    void Initialize();
    void Start(unsigned int raysPerStep);
    void Step();

    void Clear();

    struct Camera GetCamera() const;
    void SetCamera(const struct Camera);

    struct CrystalPopulation GetCrystalPopulation() const;
    void SetCrystalPopulation(const struct CrystalPopulation);

    struct LightSource GetLightSource() const;
    void SetLightSource(const struct LightSource);

    const unsigned int GetOutputTextureHandle() const;

    void ResizeOutputTextureCallback(const unsigned int width, const unsigned int height);

private:
    void InitializeShader();
    void InitializeTextures();

    unsigned int mOutputWidth;
    unsigned int mOutputHeight;
    std::mt19937 mMersenneTwister;
    std::uniform_int_distribution<unsigned int> mUniformDistribution;
    std::unique_ptr<OpenGL::Program> mSimulationShader;
    std::unique_ptr<OpenGL::Texture> mSimulationTexture;
    std::unique_ptr<OpenGL::Texture> mSpinlockTexture;

    struct Camera mCamera;
    struct CrystalPopulation mCrystals;
    struct LightSource mLight;

    bool mRunning;
    unsigned int mRaysPerStep;
    unsigned int mIteration;
};

} // namespace HaloSim
