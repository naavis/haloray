#include "crystalModel.h"
#include <QAbstractTableModel>
#include <QString>
#include <QWidget>
#include "../simulation/crystalPopulationRepository.h"

namespace HaloRay
{

CrystalModel::CrystalModel(std::shared_ptr<CrystalPopulationRepository> crystalRepository, QWidget *parent)
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

    if (role != Qt::DisplayRole && role != Qt::EditRole) return QVariant();

    auto row = index.row();
    const CrystalPopulation &crystal = m_crystals->get(row);

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
    case UpperApexAngle:
        return crystal.upperApexAngle;
    case UpperApexHeightAverage:
        return crystal.upperApexHeightAverage;
    case UpperApexHeightStd:
        return crystal.upperApexHeightStd;
    case LowerApexAngle:
        return crystal.lowerApexAngle;
    case LowerApexHeightAverage:
        return crystal.lowerApexHeightAverage;
    case LowerApexHeightStd:
        return crystal.lowerApexHeightStd;
    case PopulationWeight:
        return m_crystals->getWeight(row);
    case PopulationName:
        return QString::fromStdString(m_crystals->getName(row));
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
    CrystalPopulation &crystal = m_crystals->get(row);
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
    case UpperApexAngle:
        crystal.upperApexAngle = value.toFloat();
        break;
    case UpperApexHeightAverage:
        crystal.upperApexHeightAverage = value.toFloat();
        break;
    case UpperApexHeightStd:
        crystal.upperApexHeightStd = value.toFloat();
        break;
    case LowerApexAngle:
        crystal.lowerApexAngle = value.toFloat();
        break;
    case LowerApexHeightAverage:
        crystal.lowerApexHeightAverage = value.toFloat();
        break;
    case LowerApexHeightStd:
        crystal.lowerApexHeightStd = value.toFloat();
        break;
    case PopulationWeight:
        m_crystals->setWeight(row, value.toUInt());
        break;
    case PopulationName:
        m_crystals->setName(row, value.toString().toStdString());
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

void CrystalModel::addRow(CrystalPopulationPreset preset)
{
    auto row = m_crystals->getCount();
    beginInsertRows(QModelIndex(), row, row);
    m_crystals->add(preset);
    endInsertRows();
}

void CrystalModel::addRow(CrystalPopulation population, unsigned int weight, QString name)
{
    auto row = m_crystals->getCount();
    beginInsertRows(QModelIndex(), row, row);
    m_crystals->add(population, weight, name.toStdString());
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

void CrystalModel::clear()
{
    emit beginRemoveRows(QModelIndex(), 0, m_crystals->getCount() - 1);
    m_crystals->clear();
    endRemoveRows();
}

void CrystalModel::setName(int row, QString name)
{
    setData(createIndex(row, PopulationName), name);
}

}
