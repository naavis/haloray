#pragma once
#include <vector>
#include "crystalPopulation.h"

namespace HaloRay
{

class CrystalPopulationRepository
{
public:
    CrystalPopulationRepository();
    void add(CrystalPopulationPreset preset = Random);
    void remove(unsigned int index);
    CrystalPopulation &get(unsigned int index);
    double getProbability(unsigned int index) const;
    unsigned int getWeight(unsigned int index) const;
    void setWeight(unsigned int index, unsigned int weight);
    unsigned int getCount() const;

private:
    void addDefaults();
    std::vector<CrystalPopulation> m_crystals;
    std::vector<unsigned int> mWeights;
};

}
