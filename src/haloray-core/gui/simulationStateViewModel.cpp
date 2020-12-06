#include "simulationStateViewModel.h"

SimulationStateViewModel::SimulationStateViewModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant SimulationStateViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case SunAltitude:
            return "Sun altitude";
        case SunDiameter:
            return "Sun diameter";
        case CameraProjection:
            return "Camera projection";
        case CameraFov:
            return "Camera field-of-view";
        case CameraPitch:
            return "Camera pitch";
        case CameraYaw:
            return "Camera yaw";
        case HideSubHorizon:
            return "Hide sub-horizon";
        case DoubleScattering:
            return "Double-scattering probability";
        }
    }

    return QVariant();
}

int SimulationStateViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return 1;
}

int SimulationStateViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return NUM_COLUMNS;
}

QVariant SimulationStateViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    switch (index.column())
    {
    case SunAltitude:
    case SunDiameter:
    case CameraProjection:
    case CameraFov:
    case CameraPitch:
    case CameraYaw:
    case HideSubHorizon:
    case DoubleScattering:
    default:
        break;
        // TODO: Implement all columns
    }


    // FIXME: Implement me!
    return QVariant();
}

bool SimulationStateViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole) return false;

    if (data(index, role) != value) {
        switch (index.column())
        {
        case SunAltitude:
        case SunDiameter:
        case CameraProjection:
        case CameraFov:
        case CameraPitch:
        case CameraYaw:
        case HideSubHorizon:
        case DoubleScattering:
        default:
            break;
            // TODO: Implement all columns
        }

        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags SimulationStateViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled;
}
