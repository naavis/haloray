#pragma once
#include <vector>
#include <random>
#include "crystalPopulation.h"

namespace HaloSim
{

class CrystalPopulationRepository
{
public:
    CrystalPopulationRepository();
    bool AddDefault();
    bool Remove(unsigned int index);
    CrystalPopulation &Get(unsigned int index);
    unsigned int GetCount() const;

    const CrystalPopulation &Get();

private:
    std::vector<CrystalPopulation> mCrystals;
    std::mt19937 mMersenneTwister;
};

} // namespace HaloSim
