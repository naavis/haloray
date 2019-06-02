#pragma once
#include <random>
#include "../opengl/shader.h"
#include "../opengl/program.h"
#include "../opengl/texture.h"

namespace HaloSim
{

struct CrystalPopulation
{
    float caRatioAverage;
    float caRatioStd;

    int polarAngleDistribution;
    float polarAngleAverage;
    float polarAngleStd;

    int rotationDistribution;
    float rotationAverage;
    float rotationStd;

    bool operator==(const struct CrystalPopulation &other) const
    {
        return caRatioAverage == other.caRatioAverage &&
               caRatioStd == other.caRatioStd &&
               polarAngleDistribution == other.polarAngleDistribution &&
               polarAngleAverage == other.polarAngleAverage &&
               polarAngleStd == other.polarAngleStd &&
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
    float azimuth;
    float diameter;

    bool operator==(const struct LightSource &other) const
    {
        return altitude == other.altitude &&
               azimuth == other.azimuth &&
               diameter == other.diameter;
    }

    bool operator!=(const struct LightSource &other) const
    {
        return !operator==(other);
    }
};

struct Camera
{
    float pitch;
    float yaw;

    bool operator==(const struct Camera &other) const
    {
        return pitch == other.pitch && yaw == other.pitch;
    }

    bool operator!=(const struct Camera &other) const
    {
        return !operator==(other);
    }
};

class SimulationEngine
{
public:
    SimulationEngine();
    void Initialize();
    void Run(unsigned int numRays);
    void Clear();

    struct Camera GetCamera() const;
    void SetCamera(struct Camera);

    struct CrystalPopulation GetCrystalPopulation() const;
    void SetCrystalPopulation(struct CrystalPopulation);

    struct LightSource GetLightSource() const;
    void SetLightSource(struct LightSource);

    const unsigned int GetOutputTextureHandle() const;

private:
    void InitializeShader();
    void InitializeTextures();

    std::mt19937 mMersenneTwister;
    std::unique_ptr<OpenGL::Program> mSimulationShader;
    std::unique_ptr<OpenGL::Texture> mSimulationTexture;
    std::unique_ptr<OpenGL::Texture> mSpinlockTexture;

    struct Camera mCamera;
    struct CrystalPopulation mCrystals;
    struct LightSource mLight;
};

} // namespace HaloSim
