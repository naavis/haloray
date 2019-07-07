#include "crystalPopulationRepository.h"
#include <numeric>
#include "defaults.h"

namespace HaloSim
{

CrystalPopulationRepository::CrystalPopulationRepository()
{
    AddDefault();
}

unsigned int CrystalPopulationRepository::GetCount() const
{
    return (unsigned int)mCrystals.size();
}

bool CrystalPopulationRepository::AddDefault()
{
    mCrystals.push_back(DefaultCrystalPopulation());
    mWeights.push_back(1);
    return true;
}

bool CrystalPopulationRepository::Remove(unsigned int index)
{
    if (mCrystals.size() == 1)
        return false;

    mCrystals.erase(mCrystals.begin() + index);
    mWeights.erase(mWeights.begin() + index);

    return true;
}

CrystalPopulation &CrystalPopulationRepository::Get(unsigned int index)
{
    return mCrystals[index];
}

double CrystalPopulationRepository::GetProbability(unsigned int index) const
{
    unsigned int totalWeights = std::accumulate(mWeights.cbegin(), mWeights.cend(), 0);
    return static_cast<double>(mWeights[index]) / totalWeights;
}

unsigned int CrystalPopulationRepository::GetWeight(unsigned int index) const
{
    return mWeights[index];
}

void CrystalPopulationRepository::SetWeight(unsigned int index, unsigned int weight)
{
    mWeights[index] = weight;
}

} // namespace HaloSim
