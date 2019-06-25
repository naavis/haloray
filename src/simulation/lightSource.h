#pragma once

namespace HaloSim
{

struct LightSource
{
    float altitude;
    float diameter;

    bool operator==(const struct LightSource &other) const
    {
        return altitude == other.altitude &&
               diameter == other.diameter;
    }

    bool operator!=(const struct LightSource &other) const
    {
        return !operator==(other);
    }
};

}
