#include "crystalPopulation.h"

namespace HaloRay
{

CrystalPopulation CrystalPopulation::createLowitz()
{
    CrystalPopulation crystal;
    crystal.caRatioAverage = 1.0f;
    crystal.caRatioStd = 0.1f;

    crystal.tiltDistribution = 0;
    crystal.tiltAverage = 0.0f;
    crystal.tiltStd = 0.0f;

    crystal.rotationDistribution = 1;
    crystal.rotationAverage = 0.0f;
    crystal.rotationStd = 1.0f;

    crystal.upperApexAngle = 56.142f;
    crystal.upperApexHeightAverage = 0.0f;
    crystal.upperApexHeightStd = 0.0f;

    crystal.lowerApexAngle = 56.142f;
    crystal.lowerApexHeightAverage = 0.0f;
    crystal.lowerApexHeightStd = 0.0f;

    return crystal;
}

CrystalPopulation CrystalPopulation::createPlate()
{
    CrystalPopulation crystal;
    crystal.caRatioAverage = 0.3f;
    crystal.caRatioStd = 0.1f;

    crystal.tiltDistribution = 1;
    crystal.tiltAverage = 0.0f;
    crystal.tiltStd = 1.0f;

    crystal.rotationDistribution = 0;
    crystal.rotationAverage = 0.0f;
    crystal.rotationStd = 0.0f;

    crystal.upperApexAngle = 56.142f;
    crystal.upperApexHeightAverage = 0.0f;
    crystal.upperApexHeightStd = 0.0f;

    crystal.lowerApexAngle = 56.142f;
    crystal.lowerApexHeightAverage = 0.0f;
    crystal.lowerApexHeightStd = 0.0f;

    return crystal;
}

CrystalPopulation CrystalPopulation::createColumn()
{
    CrystalPopulation crystal;
    crystal.caRatioAverage = 7.0f;
    crystal.caRatioStd = 1.0f;

    crystal.tiltDistribution = 1;
    crystal.tiltAverage = 90.0f;
    crystal.tiltStd = 1.0f;

    crystal.rotationDistribution = 0;
    crystal.rotationAverage = 0.0f;
    crystal.rotationStd = 0.0f;

    crystal.upperApexAngle = 56.142f;
    crystal.upperApexHeightAverage = 0.0f;
    crystal.upperApexHeightStd = 0.0f;

    crystal.lowerApexAngle = 56.142f;
    crystal.lowerApexHeightAverage = 0.0f;
    crystal.lowerApexHeightStd = 0.0f;

    return crystal;
}

CrystalPopulation CrystalPopulation::createParry()
{
    CrystalPopulation crystal;
    crystal.caRatioAverage = 1.0f;
    crystal.caRatioStd = 0.1f;

    crystal.tiltDistribution = 1;
    crystal.tiltAverage = 90.0f;
    crystal.tiltStd = 1.0f;

    crystal.rotationDistribution = 1;
    crystal.rotationAverage = 0.0f;
    crystal.rotationStd = 1.0f;

    crystal.upperApexAngle = 56.142f;
    crystal.upperApexHeightAverage = 0.0f;
    crystal.upperApexHeightStd = 0.0f;

    crystal.lowerApexAngle = 56.142f;
    crystal.lowerApexHeightAverage = 0.0f;
    crystal.lowerApexHeightStd = 0.0f;

    return crystal;
}

CrystalPopulation CrystalPopulation::createRandom()
{
    CrystalPopulation crystal;
    crystal.caRatioAverage = 1.0f;
    crystal.caRatioStd = 0.1f;

    crystal.tiltDistribution = 0;
    crystal.tiltAverage = 0.0f;
    crystal.tiltStd = 0.0f;

    crystal.rotationDistribution = 0;
    crystal.rotationAverage = 0.0f;
    crystal.rotationStd = 0.0f;

    crystal.upperApexAngle = 56.142f;
    crystal.upperApexHeightAverage = 0.0f;
    crystal.upperApexHeightStd = 0.0f;

    crystal.lowerApexAngle = 56.142f;
    crystal.lowerApexHeightAverage = 0.0f;
    crystal.lowerApexHeightStd = 0.0f;

    return crystal;
}

CrystalPopulation CrystalPopulation::createPyramid()
{
    CrystalPopulation crystal;
    crystal.caRatioAverage = 0.1f;
    crystal.caRatioStd = 0.1f;

    crystal.tiltDistribution = 0;
    crystal.tiltAverage = 0.0f;
    crystal.tiltStd = 0.0f;

    crystal.rotationDistribution = 0;
    crystal.rotationAverage = 0.0f;
    crystal.rotationStd = 0.0f;

    crystal.upperApexAngle = 56.142f;
    crystal.upperApexHeightAverage = 0.5f;
    crystal.upperApexHeightStd = 0.1f;

    crystal.lowerApexAngle = 56.142f;
    crystal.lowerApexHeightAverage = 0.5f;
    crystal.lowerApexHeightStd = 0.1f;

    return crystal;
}

void CrystalPopulation::initializePrismFaceDistances()
{
    for (auto i = 0u; i < 6; ++i)
    {
        prismFaceDistances[i] = 1.0f;
    }
}

CrystalPopulation::CrystalPopulation()
{
    enabled = true;
    initializePrismFaceDistances();
}

CrystalPopulation CrystalPopulation::presetPopulation(CrystalPopulationPreset preset)
{
    switch (preset)
    {
    case Random:
        return CrystalPopulation::createRandom();
    case Plate:
        return CrystalPopulation::createPlate();
    case Column:
        return CrystalPopulation::createColumn();
    case Parry:
        return CrystalPopulation::createParry();
    case Lowitz:
        return CrystalPopulation::createLowitz();
    case Pyramid:
        return CrystalPopulation::createPyramid();
    default:
        return CrystalPopulation::createRandom();
    }
}

}
