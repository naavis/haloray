#include "camera.h"

namespace HaloRay
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

bool Camera::operator==(const Camera &other) const
{
    return pitch == other.pitch
            && yaw == other.yaw
            && fov == other.fov
            && projection == other.projection
            && hideSubHorizon == other.hideSubHorizon;
}

bool Camera::operator!=(const Camera &other) const
{
    return !(*this == other);
}

Camera Camera::createDefaultCamera()
{
    Camera camera;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.fov = 75.0f;
    camera.projection = Projection::Stereographic;
    camera.hideSubHorizon = false;
    return camera;
}

}
