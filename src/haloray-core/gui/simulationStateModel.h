#pragma once

#include <QAbstractTableModel>
#include "simulation/camera.h"

namespace HaloRay {
class SimulationEngine;

class SimulationStateModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SimulationStateModel(SimulationEngine *engine, QObject *parent = nullptr);

    enum Columns
    {
        SunAltitude,
        SunDiameter,
        CameraProjection,
        CameraFov,
        CameraMaxFov,
        CameraPitch,
        CameraYaw,
        HideSubHorizon,
        MultipleScatteringProbability,
        RaysPerFrame,
        MaximumIterations,
        RaysPerFrameUpperLimit,
        AtmosphereEnabled,
        Turbidity,
        GroundAlbedo,
        NUM_COLUMNS
    };

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Convenience methods
    void setRaysPerFrame(unsigned int maxRaysPerFrame);
    void setRaysPerFrameUpperLimit(unsigned int upperLimit);
    unsigned int getRaysPerFrameUpperLimit() const;
    unsigned int getMaxIterations() const;

private:
    SimulationEngine *m_simulationEngine;
    void setSunAltitude(float altitude);
    void setSunDiameter(float diameter);
    void setCameraProjection(Projection projection);
    void setCameraFov(float fov);
    void setCameraPitch(float pitch);
    void setCameraYaw(float yaw);
    void setHideSubHorizon(bool hide);

    unsigned int m_maximumIterations;
    unsigned int m_raysPerFrameUpperLimit;
};

}
