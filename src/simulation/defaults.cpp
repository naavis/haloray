#include "defaults.h"

namespace HaloSim
{

LightSource DefaultLightSource()
{
    LightSource sun;
    sun.altitude = 30.0f;
    sun.diameter = 0.5f;
    return sun;
}

Camera DefaultCamera()
{
    Camera camera;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.fov = 0.5f;
    camera.projection = HaloSim::Projection::Stereographic;
    camera.hideSubHorizon = false;
    return camera;
}

CrystalPopulation DefaultCrystalPopulation()
{
    // Defaults to a Lowitz display
    CrystalPopulation crystalProperties;
    crystalProperties.caRatioAverage = 0.3f;
    crystalProperties.caRatioStd = 0.0f;

    crystalProperties.tiltDistribution = 1;
    crystalProperties.tiltAverage = 0.0f;
    crystalProperties.tiltStd = 40.0f;

    crystalProperties.rotationDistribution = 1;
    crystalProperties.rotationAverage = 0.0f;
    crystalProperties.rotationStd = 1.0f;

    return crystalProperties;
}

} // namespace HaloSim
