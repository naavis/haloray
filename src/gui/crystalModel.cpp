#include "crystalModel.h"
#include <QDebug>
#include "../simulation/defaults.h"

CrystalModel::CrystalModel(QWidget *parent)
    : QAbstractTableModel(parent)
{
    auto pop1 = HaloSim::DefaultCrystalPopulation();
    pop1.caRatioAverage = 2.0f;
    pop1.tiltAverage = 90.0f;
    pop1.rotationDistribution = 1;
    pop1.tiltDistribution = 0;
    mCrystals.push_back(pop1);

    auto pop2 = HaloSim::DefaultCrystalPopulation();
    pop2.caRatioAverage = 0.5f;
    pop2.tiltAverage = 0.0f;
    pop2.rotationDistribution = 1;
    mCrystals.push_back(pop2);
}

int CrystalModel::rowCount(const QModelIndex &parent) const
{
    return static_cast<int>(mCrystals.size());
}

int CrystalModel::columnCount(const QModelIndex &parent) const
{
    return 8;
}

QVariant CrystalModel::data(const QModelIndex &index, int role) const
{
    qDebug() << "Read data with index" << index.row() << index.column() << "and role" << role;
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    auto col = index.column();
    auto row = index.row();
    const HaloSim::CrystalPopulation &crystal = mCrystals[row];

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
    qDebug() << "Read data with index" << index.row() << index.column() << "and role" << role;
    if (role != Qt::EditRole)
        return false;

    if (!checkIndex(index))
        return false;

    auto row = index.row();
    auto col = index.column();
    HaloSim::CrystalPopulation &crystal = mCrystals[row];

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
    return true;
}

Qt::ItemFlags CrystalModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}
