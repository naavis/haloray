#include "camera.h"

namespace HaloSim
{

float Camera::getMaximumFov() const
{
    switch (projection)
    {
    case Stereographic:
        return 350.0f;
    case Rectilinear:
        return 160.0f;
    case Orthographic:
        return 180.0f;
    default:
        return 360.0f;
    }
}

Camera Camera::createDefaultCamera()
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
