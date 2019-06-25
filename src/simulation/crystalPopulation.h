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

    bool operator==(const struct CrystalPopulation &other) const
    {
        return caRatioAverage == other.caRatioAverage &&
               caRatioStd == other.caRatioStd &&
               tiltDistribution == other.tiltDistribution &&
               tiltAverage == other.tiltAverage &&
               tiltStd == other.tiltStd &&
               rotationDistribution == other.rotationDistribution &&
               rotationAverage == other.rotationAverage &&
               rotationStd == other.rotationStd;
    }

    bool operator!=(const struct CrystalPopulation &other) const
    {
        return !operator==(other);
    }
};

}
