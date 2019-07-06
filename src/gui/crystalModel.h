#pragma once
#include <QAbstractTableModel>
#include <QWidget>
#include <vector>
#include <memory>
#include "../simulation/crystalPopulation.h"
#include "../simulation/crystalPopulationRepository.h"

class CrystalModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CrystalModel(std::shared_ptr<HaloSim::CrystalPopulationRepository> crystalRepository, QWidget *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addRow();
    void removeRow(int row);

private:
    std::shared_ptr<HaloSim::CrystalPopulationRepository> mCrystals;
};
