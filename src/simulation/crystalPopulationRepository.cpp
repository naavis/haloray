#include "crystalPopulationRepository.h"
#include <numeric>

namespace HaloSim
{

CrystalPopulationRepository::CrystalPopulationRepository()
{
    addDefaults();
}

unsigned int CrystalPopulationRepository::getCount() const
{
    return (unsigned int)mCrystals.size();
}

void CrystalPopulationRepository::add()
{
    mCrystals.push_back(CrystalPopulation::createRandom());
    mWeights.push_back(1);
}

void CrystalPopulationRepository::addDefaults()
{
    mCrystals.push_back(CrystalPopulation::createColumn());
    mWeights.push_back(1);

    mCrystals.push_back(CrystalPopulation::createPlate());
    mWeights.push_back(1);

    mCrystals.push_back(CrystalPopulation::createRandom());
    mWeights.push_back(1);
}

void CrystalPopulationRepository::remove(unsigned int index)
{
    mCrystals.erase(mCrystals.begin() + index);
    mWeights.erase(mWeights.begin() + index);
}

CrystalPopulation &CrystalPopulationRepository::get(unsigned int index)
{
    return mCrystals[index];
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

} // namespace HaloSim
