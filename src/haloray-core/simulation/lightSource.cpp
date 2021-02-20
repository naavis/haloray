#include "lightSource.h"

namespace HaloRay
{

bool LightSource::operator==(const LightSource &other) const
{
    return altitude == other.altitude
            && diameter == other.diameter;
}

bool LightSource::operator!=(const LightSource &other) const
{
    return !(*this == other);
}

LightSource LightSource::createDefaultLightSource()
{
    LightSource sun;
    sun.altitude = 30.0f;
    sun.diameter = 0.5f;
    return sun;
}

}
