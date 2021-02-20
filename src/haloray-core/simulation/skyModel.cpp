#include "skyModel.h"
#include <algorithm>
#include "hosekWilkie/ArHosekSkyModel.h"
#include "colorUtilities.h"

namespace HaloRay
{

SkyModel ConvertSkyModelState(ArHosekSkyModelState *originalState)
{
    SkyModel state;
    state.turbidity = (float)originalState->turbidity;
    for (auto channel = 0u; channel < 3; ++channel)
    {
        state.radiances[channel] = (float)(originalState->radiances[channel]);
        for (auto config = 0u; config < 9; ++config)
        {
            state.configs[channel][config] = (float)(originalState->configs[channel][config]);
        }
    }

    return state;
}

SkyModel::SkyModel()
{
}

struct SkyModel SkyModel::Create(double solarElevation, double atmosphericTurbidity, double groundAlbedo, double solarRadius)
{
    auto tempSkyState = arhosek_xyz_skymodelstate_alloc_init(atmosphericTurbidity, groundAlbedo, solarElevation, solarRadius);
    auto state = ConvertSkyModelState(tempSkyState);
    arhosekskymodelstate_free(tempSkyState);

    auto tempSunState = arhosekskymodelstate_alloc_init(solarElevation, atmosphericTurbidity, groundAlbedo, solarRadius);

    float solarRadianceTop[31];
    float solarRadianceBottom[31];
    float solarRadianceTopWithLimbDarkening[31];

    for (auto i = 0u; i < 31; ++i)
    {
        float wl = cieWl[i];
        solarRadianceTop[i] = (float)arhosekskymodel_solar_radiance_plain(tempSunState, solarElevation + solarRadius, (double)wl);
        solarRadianceBottom[i] = (float)arhosekskymodel_solar_radiance_plain(tempSunState, std::max(solarElevation - solarRadius, 0.0), (double)wl);
        solarRadianceTopWithLimbDarkening[i] = (float)arhosekskymodel_solar_radiance_with_limb_darkening(tempSunState, (double)wl,  solarElevation + solarRadius, solarRadius);
        state.sunSpectrum[i] = (float)arhosekskymodel_solar_radiance_plain(tempSunState, std::max(solarElevation, 0.0), (double)wl) / 30663.7f;
    }

    arhosekskymodelstate_free(tempSunState);

    state.sunTopCIEXYZ[0] = getCIEX(solarRadianceTop);
    state.sunTopCIEXYZ[1] = getCIEY(solarRadianceTop);
    state.sunTopCIEXYZ[2] = getCIEZ(solarRadianceTop);

    state.sunBottomCIEXYZ[0] = getCIEX(solarRadianceBottom);
    state.sunBottomCIEXYZ[1] = getCIEY(solarRadianceBottom);
    state.sunBottomCIEXYZ[2] = getCIEZ(solarRadianceBottom);

    state.limbDarkeningScaler[0] = getCIEX(solarRadianceTopWithLimbDarkening) / state.sunTopCIEXYZ[0];
    state.limbDarkeningScaler[1] = getCIEY(solarRadianceTopWithLimbDarkening) / state.sunTopCIEXYZ[1];
    state.limbDarkeningScaler[2] = getCIEZ(solarRadianceTopWithLimbDarkening) / state.sunTopCIEXYZ[2];

    return state;
}

}
