#pragma once

namespace HaloRay
{

enum CrystalPopulationPreset
{
    Random,
    Plate,
    Column,
    Parry,
    Lowitz
};

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

    float upperApexAngle;
    float upperApexHeightAverage;
    float upperApexHeightStd;

    float lowerApexAngle;
    float lowerApexHeightAverage;
    float lowerApexHeightStd;

    static CrystalPopulation presetPopulation(CrystalPopulationPreset);
    static CrystalPopulation createLowitz();
    static CrystalPopulation createPlate();
    static CrystalPopulation createColumn();
    static CrystalPopulation createParry();
    static CrystalPopulation createRandom();
};

}
