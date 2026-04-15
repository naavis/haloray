#pragma once

namespace HaloRay
{

struct Atmosphere
{
    bool enabled;
    double turbidity;
    double groundAlbedo;

    bool operator==(const Atmosphere&) const;
    bool operator!=(const Atmosphere&) const;

    static Atmosphere createDefaultAtmosphere();
};

}
