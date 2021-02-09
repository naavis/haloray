#include <cmath>
#include "trigonometryUtilities.h"
#include "camera.h"

namespace HaloRay
{

float Camera::getFocalLength() const
{
    float fovRadian = degToRad(fov);
    switch (projection)
    {
    case Stereographic:
        return 1.0f / (4.0f * tan(fovRadian / 4.0));
    case Rectilinear:
        return 1.0 / (2.0 * tan(fovRadian / 2.0));
    case Equidistant:
        return 1.0 / fovRadian;
    case EqualArea:
        return 1.0 / (4.0 * sin(fovRadian / 4.0));
    case Orthographic:
        return 1.0 / (2.0 * sin(fovRadian / 2.0));
    }

    return 1.0;
}

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
    camera.pitch = 30.0f;
    camera.yaw = 15.0f;
    camera.fov = 75.0f;
    camera.projection = Projection::Stereographic;
    camera.hideSubHorizon = true;
    return camera;
}

}
