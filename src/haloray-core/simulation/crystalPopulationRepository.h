#pragma once
#include <vector>
#include <string>
#include "crystalPopulation.h"

namespace HaloRay
{

class CrystalPopulationRepository
{
public:
    CrystalPopulationRepository();
    void add(CrystalPopulationPreset preset = Random);
    void add(CrystalPopulation population, double weight, std::string name);
    void remove(unsigned int index);
    void clear();

    CrystalPopulation &get(unsigned int index);

    void setName(unsigned int index, std::string name);
    std::string getName(unsigned int index) const;

    double getWeight(unsigned int index) const;
    void setWeight(unsigned int index, double weight);
    double getProbability(unsigned int index) const;

    unsigned int getCount() const;

private:
    void addDefaults();
    std::vector<CrystalPopulation> m_crystals;
    std::vector<double> m_weights;
    std::vector<std::string> m_names;
    unsigned int m_nextId;
};

}
