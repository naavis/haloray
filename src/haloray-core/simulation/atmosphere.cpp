#include "atmosphere.h"

namespace HaloRay
{

bool Atmosphere::operator==(const Atmosphere &other) const
{
    return enabled == other.enabled
            && turbidity == other.turbidity
            && groundAlbedo == other.groundAlbedo;
}

bool Atmosphere::operator!=(const Atmosphere &other) const
{
    return !(*this == other);
}

Atmosphere Atmosphere::createDefaultAtmosphere()
{
    Atmosphere a;
    a.enabled = true;
    a.turbidity = 5.0f;
    a.groundAlbedo = 0.0f;
    return a;
}

}

