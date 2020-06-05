#pragma once

namespace HaloRay
{

struct LightSource
{
    float altitude;
    float diameter;

    static LightSource createDefaultLightSource();
};

} // namespace HaloRay
