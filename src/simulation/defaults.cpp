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
    camera.fov = 75.0f;
    camera.projection = HaloSim::Projection::Stereographic;
    camera.hideSubHorizon = false;
    return camera;
}

} // namespace HaloSim
