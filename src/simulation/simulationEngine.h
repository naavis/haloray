#pragma once
#include <random>
#include <memory>
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
    EqualArea
};

struct Camera
{
    float pitch;
    float yaw;
    float fov;
    Projection projection;

    bool operator==(const struct Camera &other) const
    {
        return pitch == other.pitch && yaw == other.yaw && fov == other.fov && projection == other.projection;
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
    void SetCamera(const struct Camera);

    struct CrystalPopulation GetCrystalPopulation() const;
    void SetCrystalPopulation(const struct CrystalPopulation);

    struct LightSource GetLightSource() const;
    void SetLightSource(const struct LightSource);

    const unsigned int GetOutputTextureHandle() const;

private:
    void InitializeShader();
    void InitializeTextures();

    std::mt19937 mMersenneTwister;
    std::uniform_int_distribution<unsigned int> mUniformDistribution;
    std::unique_ptr<OpenGL::Program> mSimulationShader;
    std::unique_ptr<OpenGL::Texture> mSimulationTexture;
    std::unique_ptr<OpenGL::Texture> mSpinlockTexture;

    struct Camera mCamera;
    struct CrystalPopulation mCrystals;
    struct LightSource mLight;
};

} // namespace HaloSim
