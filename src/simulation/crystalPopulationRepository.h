#pragma once
#include <vector>
#include "crystalPopulation.h"

namespace HaloSim
{

class CrystalPopulationRepository
{
public:
    CrystalPopulationRepository();
    void addDefault();
    void remove(unsigned int index);
    CrystalPopulation &get(unsigned int index);
    double getProbability(unsigned int index) const;
    unsigned int getWeight(unsigned int index) const;
    void setWeight(unsigned int index, unsigned int weight);
    unsigned int getCount() const;

private:
    std::vector<CrystalPopulation> mCrystals;
    std::vector<unsigned int> mWeights;
};

} // namespace HaloSim
