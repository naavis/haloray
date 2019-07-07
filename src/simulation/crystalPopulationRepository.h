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
    double GetProbability(unsigned int index) const;
    unsigned int GetWeight(unsigned int index) const;
    void SetWeight(unsigned int index, unsigned int weight);
    unsigned int GetCount() const;

private:
    std::vector<CrystalPopulation> mCrystals;
    std::vector<unsigned int> mWeights;
};

} // namespace HaloSim
