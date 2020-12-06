#pragma once

#include <QAbstractTableModel>

class SimulationStateViewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SimulationStateViewModel(QObject *parent = nullptr);

    enum
    {
        SunAltitude,
        SunDiameter,
        CameraProjection,
        CameraFov,
        CameraPitch,
        CameraYaw,
        HideSubHorizon,
        DoubleScattering,
        NUM_COLUMNS
    } Columns;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
};

