#include "crystalPopulationRepository.h"
#include <QString>
#include <numeric>

namespace HaloRay
{

CrystalPopulationRepository::CrystalPopulationRepository()
    : m_nextId(1)
{
    addDefaults();
}

unsigned int CrystalPopulationRepository::getCount() const
{
    return (unsigned int)m_crystals.size();
}

void CrystalPopulationRepository::add(CrystalPopulationPreset preset)
{
    QString presetNameMap[6] = {"Random", "Plate", "Column", "Parry", "Lowitz", "Pyramid"};
    m_crystals.push_back(CrystalPopulation::presetPopulation(preset));
    m_names.push_back(QString("%1 population %2").arg(presetNameMap[preset]).arg(m_nextId).toStdString());
    m_nextId = m_nextId + 1;
    m_weights.push_back(1.0);
}

void CrystalPopulationRepository::add(CrystalPopulation population, double weight, std::string name)
{
    m_crystals.push_back(population);
    m_names.push_back(name);
    m_weights.push_back(weight);
}

void CrystalPopulationRepository::addDefaults()
{
    m_crystals.push_back(CrystalPopulation::presetPopulation(CrystalPopulationPreset::Column));
    m_names.push_back("Column");
    m_weights.push_back(1.0);

    m_crystals.push_back(CrystalPopulation::presetPopulation(CrystalPopulationPreset::Plate));
    m_names.push_back("Plate");
    m_weights.push_back(1.0);

    m_crystals.push_back(CrystalPopulation::presetPopulation(CrystalPopulationPreset::Random));
    m_names.push_back("Random");
    m_weights.push_back(1.0);
}

void CrystalPopulationRepository::remove(unsigned int index)
{
    m_crystals.erase(m_crystals.begin() + index);
    m_names.erase(m_names.begin() + index);
    m_weights.erase(m_weights.begin() + index);
}

void CrystalPopulationRepository::clear()
{
    m_crystals.clear();
    m_names.clear();
    m_weights.clear();
}

CrystalPopulation &CrystalPopulationRepository::get(unsigned int index)
{
    return m_crystals[index];
}

void CrystalPopulationRepository::setName(unsigned int index, std::string name)
{
    m_names[index] = name;
}

std::string CrystalPopulationRepository::getName(unsigned int index) const
{
    return m_names[index];
}

double CrystalPopulationRepository::getProbability(unsigned int index) const
{
    if (m_crystals[index].enabled == false) return 0.0;

    auto totalWeights = 0.0;
    for (auto i = 0u; i < getCount(); ++i)
    {
        totalWeights += m_crystals[i].enabled ? m_weights[i] : 0.0;
    }
    return m_weights[index] / totalWeights;
}

double CrystalPopulationRepository::getWeight(unsigned int index) const
{
    return m_weights[index];
}

void CrystalPopulationRepository::setWeight(unsigned int index, double weight)
{
    m_weights[index] = weight;
}

}
