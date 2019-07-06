#include "crystalPopulationRepository.h"
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
    return true;
}

bool CrystalPopulationRepository::Remove(unsigned int index)
{
    if (mCrystals.size() == 1)
        return false;

    mCrystals.erase(mCrystals.begin() + index);

    return true;
}

CrystalPopulation &CrystalPopulationRepository::Get(unsigned int index)
{
    return mCrystals[index];
}

} // namespace HaloSim
