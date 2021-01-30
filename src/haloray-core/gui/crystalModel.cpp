#include "crystalModel.h"
#include <QAbstractTableModel>
#include <QWidget>
#include "../simulation/crystalPopulationRepository.h"

namespace HaloRay
{

CrystalModel::CrystalModel(std::shared_ptr<HaloRay::CrystalPopulationRepository> crystalRepository, QWidget *parent)
    : QAbstractTableModel(parent),
      m_crystals(crystalRepository)
{
}

int CrystalModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return static_cast<int>(m_crystals->getCount());
}

int CrystalModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return NUM_COLUMNS;
}

QVariant CrystalModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto row = index.row();
    const HaloRay::CrystalPopulation &crystal = m_crystals->get(row);

    switch (index.column())
    {
    case CaRatioAverage:
        return crystal.caRatioAverage;
    case CaRatioStd:
        return crystal.caRatioStd;
    case TiltDistribution:
        return crystal.tiltDistribution;
    case TiltAverage:
        return crystal.tiltAverage;
    case TiltStd:
        return crystal.tiltStd;
    case RotationDistribution:
        return crystal.rotationDistribution;
    case RotationAverage:
        return crystal.rotationAverage;
    case RotationStd:
        return crystal.rotationStd;
    case PopulationWeight:
        return m_crystals->getWeight(row);
    }

    return QVariant();
}

bool CrystalModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!checkIndex(index))
        return false;

    if (data(index, role) == value) return false;

    auto row = index.row();
    HaloRay::CrystalPopulation &crystal = m_crystals->get(row);
    switch (index.column())
    {
    case CaRatioAverage:
        crystal.caRatioAverage = value.toFloat();
        break;
    case CaRatioStd:
        crystal.caRatioStd = value.toFloat();
        break;
    case TiltDistribution:
        crystal.tiltDistribution = value.toInt();
        break;
    case TiltAverage:
        crystal.tiltAverage = value.toFloat();
        break;
    case TiltStd:
        crystal.tiltStd = value.toFloat();
        break;
    case RotationDistribution:
        crystal.rotationDistribution = value.toInt();
        break;
    case RotationAverage:
        crystal.rotationAverage = value.toFloat();
        break;
    case RotationStd:
        crystal.rotationStd = value.toFloat();
        break;
    case PopulationWeight:
        m_crystals->setWeight(row, value.toUInt());
        break;
    default:
        break;
    }

    emit dataChanged(index, index);

    return true;
}

Qt::ItemFlags CrystalModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

void CrystalModel::addRow(HaloRay::CrystalPopulationPreset preset)
{
    auto row = m_crystals->getCount();
    beginInsertRows(QModelIndex(), row, row);
    m_crystals->add(preset);
    endInsertRows();
}

bool CrystalModel::removeRow(int row)
{
    if (m_crystals->getCount() <= 1)
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_crystals->remove(row);
    endRemoveRows();

    return true;
}

}
