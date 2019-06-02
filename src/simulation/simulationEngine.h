#pragma once
#include "../opengl/shader.h"
#include "../opengl/program.h"
#include "../opengl/texture.h"

namespace HaloSim
{

typedef struct
{
    float caRatioAverage;
    float caRatioStd;

    int polarAngleDistribution;
    float polarAngleAverage;
    float polarAngleStd;

    int rotationDistribution;
    float rotationAverage;
    float rotationStd;
} crystalProperties_t;

typedef struct
{
    float altitude;
    float azimuth;
    float diameter;
} sunProperties_t;

class SimulationEngine
{
public:
    void Initialize();
    void Run(crystalProperties_t crystal, sunProperties_t sun, unsigned int numRays) const;
    const unsigned int GetOutputTextureHandle() const;

private:
    void InitializeShader();
    void InitializeTextures();
    std::unique_ptr<OpenGL::Program> mSimulationShader;
    std::unique_ptr<OpenGL::Texture> mSimulationTexture;
    std::unique_ptr<OpenGL::Texture> mSpinlockTexture;
};

} // namespace HaloSim
