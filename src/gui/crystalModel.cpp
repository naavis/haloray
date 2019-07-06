#include "crystalModel.h"
#include "../simulation/defaults.h"

CrystalModel::CrystalModel(std::shared_ptr<HaloSim::CrystalPopulationRepository> crystalRepository, QWidget *parent)
    : QAbstractTableModel(parent),
      mCrystals(crystalRepository)
{
}

int CrystalModel::rowCount(const QModelIndex &parent) const
{
    return static_cast<int>(mCrystals->GetCount());
}

int CrystalModel::columnCount(const QModelIndex &parent) const
{
    return 8;
}

QVariant CrystalModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto col = index.column();
    auto row = index.row();
    const HaloSim::CrystalPopulation &crystal = mCrystals->Get(row);

    switch (col)
    {
    case 0:
        return crystal.caRatioAverage;
    case 1:
        return crystal.caRatioStd;
    case 2:
        return crystal.tiltDistribution;
    case 3:
        return crystal.tiltAverage;
    case 4:
        return crystal.tiltStd;
    case 5:
        return crystal.rotationDistribution;
    case 6:
        return crystal.rotationAverage;
    case 7:
        return crystal.rotationStd;
    }

    return QVariant();
}

bool CrystalModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!checkIndex(index))
        return false;

    auto row = index.row();
    auto col = index.column();
    HaloSim::CrystalPopulation &crystal = mCrystals->Get(row);
    switch (col)
    {
    case 0:
        crystal.caRatioAverage = value.toFloat();
        break;
    case 1:
        crystal.caRatioStd = value.toFloat();
        break;
    case 2:
        crystal.tiltDistribution = value.toInt();
        break;
    case 3:
        crystal.tiltAverage = value.toFloat();
        break;
    case 4:
        crystal.tiltStd = value.toFloat();
        break;
    case 5:
        crystal.rotationDistribution = value.toInt();
        break;
    case 6:
        crystal.rotationAverage = value.toFloat();
        break;
    case 7:
        crystal.rotationStd = value.toFloat();
        break;
    default:
        break;
    }

    emit dataChanged(createIndex(0, 0), createIndex(0, 0), {Qt::DisplayRole});

    return true;
}

Qt::ItemFlags CrystalModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

void CrystalModel::addRow()
{
    auto row = mCrystals->GetCount();
    beginInsertRows(QModelIndex(), row, row);

    bool success = mCrystals->AddDefault();

    endInsertRows();
}

bool CrystalModel::removeRow(int row)
{
    beginRemoveRows(QModelIndex(), row, row);

    bool success = mCrystals->Remove(row);

    endRemoveRows();

    return success;
}
