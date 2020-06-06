#include "crystalPopulationRepository.h"
#include <numeric>

namespace HaloRay
{

CrystalPopulationRepository::CrystalPopulationRepository()
{
    addDefaults();
}

unsigned int CrystalPopulationRepository::getCount() const
{
    return (unsigned int)m_crystals.size();
}

void CrystalPopulationRepository::add(CrystalPopulationPreset preset)
{
    m_crystals.push_back(CrystalPopulation::presetPopulation(preset));
    mWeights.push_back(1);
}

void CrystalPopulationRepository::addDefaults()
{
    m_crystals.push_back(CrystalPopulation::presetPopulation(CrystalPopulationPreset::Column));
    mWeights.push_back(1);

    m_crystals.push_back(CrystalPopulation::presetPopulation(CrystalPopulationPreset::Plate));
    mWeights.push_back(1);

    m_crystals.push_back(CrystalPopulation::presetPopulation(CrystalPopulationPreset::Random));
    mWeights.push_back(1);
}

void CrystalPopulationRepository::remove(unsigned int index)
{
    m_crystals.erase(m_crystals.begin() + index);
    mWeights.erase(mWeights.begin() + index);
}

CrystalPopulation &CrystalPopulationRepository::get(unsigned int index)
{
    return m_crystals[index];
}

double CrystalPopulationRepository::getProbability(unsigned int index) const
{
    unsigned int totalWeights = std::accumulate(mWeights.cbegin(), mWeights.cend(), 0);
    return static_cast<double>(mWeights[index]) / totalWeights;
}

unsigned int CrystalPopulationRepository::getWeight(unsigned int index) const
{
    return mWeights[index];
}

void CrystalPopulationRepository::setWeight(unsigned int index, unsigned int weight)
{
    mWeights[index] = weight;
}

}
