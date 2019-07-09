#pragma once

namespace HaloSim
{

struct LightSource
{
    float altitude;
    float diameter;

    static LightSource createDefaultLightSource();
};

} // namespace HaloSim
