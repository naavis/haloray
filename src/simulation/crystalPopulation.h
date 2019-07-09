#pragma once

namespace HaloSim
{

struct CrystalPopulation
{
    float caRatioAverage;
    float caRatioStd;

    int tiltDistribution;
    float tiltAverage;
    float tiltStd;

    int rotationDistribution;
    float rotationAverage;
    float rotationStd;

    static CrystalPopulation createLowitz();
    static CrystalPopulation createPlate();
    static CrystalPopulation createColumn();
    static CrystalPopulation createParry();
    static CrystalPopulation createRandom();
};

} // namespace HaloSim
