#include "crystalPopulation.h"

namespace HaloSim
{

CrystalPopulation CrystalPopulation::createLowitz()
{
    CrystalPopulation crystalProperties;
    crystalProperties.caRatioAverage = 0.3f;
    crystalProperties.caRatioStd = 0.0f;

    crystalProperties.tiltDistribution = 0;
    crystalProperties.tiltAverage = 0.0f;
    crystalProperties.tiltStd = 0.0f;

    crystalProperties.rotationDistribution = 1;
    crystalProperties.rotationAverage = 0.0f;
    crystalProperties.rotationStd = 1.0f;

    return crystalProperties;
}

} // namespace HaloSim
