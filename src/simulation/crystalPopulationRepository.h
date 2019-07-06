#pragma once
#include <vector>
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

private:
    std::vector<CrystalPopulation> mCrystals;
};

} // namespace HaloSim
