#pragma once
#include <QAbstractTableModel>
#include <memory>
#include "simulation/crystalPopulation.h"

class QWidget;
class QVariant;
class QModelIndex;
class QString;

namespace HaloRay
{

class CrystalPopulationRepository;

class CrystalModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CrystalModel(std::shared_ptr<CrystalPopulationRepository> crystalRepository, QWidget *parent = nullptr);

    enum Columns
    {
        CaRatioAverage,
        CaRatioStd,
        TiltDistribution,
        TiltAverage,
        TiltStd,
        RotationDistribution,
        RotationAverage,
        RotationStd,
        UpperApexAngle,
        UpperApexHeightAverage,
        UpperApexHeightStd,
        LowerApexAngle,
        LowerApexHeightAverage,
        LowerApexHeightStd,
        PopulationWeight,
        PopulationName,
        PrismFaceDistance1,
        PrismFaceDistance2,
        PrismFaceDistance3,
        PrismFaceDistance4,
        PrismFaceDistance5,
        PrismFaceDistance6,
        NUM_COLUMNS
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addRow(CrystalPopulationPreset preset = CrystalPopulationPreset::Random);
    void addRow(CrystalPopulation population, unsigned int weight, QString name);
    bool removeRow(int row);
    void clear();
    void setName(int row, QString name);

private:
    std::shared_ptr<CrystalPopulationRepository> m_crystals;
};

}
