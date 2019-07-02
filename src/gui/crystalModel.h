#pragma once
#include <QAbstractTableModel>
#include <QWidget>
#include <vector>
#include "../simulation/crystalPopulation.h"

class CrystalModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CrystalModel(QWidget *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    std::vector<HaloSim::CrystalPopulation> mCrystals;
};
