#include "lightSource.h"

namespace HaloRay
{

LightSource LightSource::createDefaultLightSource()
{
    LightSource sun;
    sun.altitude = 30.0f;
    sun.diameter = 0.5f;
    return sun;
}

}
