#pragma once
#include <QAbstractTableModel>
#include <memory>
#include "../simulation/crystalPopulation.h"

class QWidget;
class QVariant;
class QModelIndex;

namespace HaloRay
{

class CrystalPopulationRepository;

class CrystalModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CrystalModel(std::shared_ptr<HaloRay::CrystalPopulationRepository> crystalRepository, QWidget *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addRow(HaloRay::CrystalPopulationPreset preset = HaloRay::CrystalPopulationPreset::Random);
    bool removeRow(int row);

private:
    std::shared_ptr<HaloRay::CrystalPopulationRepository> m_crystals;
};

}
