#pragma once

namespace HaloRay
{

struct SkyModel
{
    float configs[3][9];
    float radiances[3];
    float turbidity;
    float sunTopCIEXYZ[3];
    float sunBottomCIEXYZ[3];
    float limbDarkeningScaler[3];
    float sunSpectrum[31];

    SkyModel();
    static struct SkyModel Create(double solarElevation, double atmosphericTurbidity, double groundAlbedo, double solarRadius);
};

}
