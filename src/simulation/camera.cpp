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

} // namespace HaloSim
