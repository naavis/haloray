#include "simulationStateModel.h"
#include "simulation/simulationEngine.h"

namespace HaloRay
{

SimulationStateModel::SimulationStateModel(SimulationEngine *engine, QObject *parent)
    : QAbstractTableModel(parent),
      m_simulationEngine(engine),
      m_maximumIterations(600)
{
    connect(m_simulationEngine, &SimulationEngine::cameraChanged, [this]() {
        emit dataChanged(createIndex(0, CameraProjection), createIndex(0, HideSubHorizon));
    });

    connect(m_simulationEngine, &SimulationEngine::lightSourceChanged, [this]() {
        emit dataChanged(createIndex(0, SunAltitude), createIndex(0, SunDiameter));
    });

    connect(m_simulationEngine, &SimulationEngine::lockCameraToLightSourceChanged, [this]() {
        emit dataChanged(createIndex(0, CameraProjection), createIndex(0, HideSubHorizon));
    });

    connect(m_simulationEngine, &SimulationEngine::multipleScatteringProbabilityChanged, [this]() {
        emit dataChanged(createIndex(0, MultipleScatteringProbability), createIndex(0, MultipleScatteringProbability));
    });
}

QVariant SimulationStateModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case CameraMaxFov:
            return "Camera maximum field-of-view";
        case CameraPitch:
            return "Camera pitch";
        case CameraYaw:
            return "Camera yaw";
        case HideSubHorizon:
            return "Hide sub-horizon";
        case MultipleScatteringProbability:
            return "Multiple scattering probability";
        case MaximumRaysPerFrame:
            return "Maximum rays per frame";
        case MaximumIterations:
            return "Maximum iterations";
        }
    }

    return QVariant();
}

int SimulationStateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return 1;
}

int SimulationStateModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return NUM_COLUMNS;
}

QVariant SimulationStateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    switch (index.column())
    {
    case SunAltitude:
        return m_simulationEngine->getLightSource().altitude;
    case SunDiameter:
        return m_simulationEngine->getLightSource().diameter;
    case CameraProjection:
        return m_simulationEngine->getCamera().projection;
    case CameraFov:
        return m_simulationEngine->getCamera().fov;
    case CameraMaxFov:
        return m_simulationEngine->getCamera().getMaximumFov();
    case CameraPitch:
        return m_simulationEngine->getCamera().pitch;
    case CameraYaw:
        return m_simulationEngine->getCamera().yaw;
    case HideSubHorizon:
        return m_simulationEngine->getCamera().hideSubHorizon;
    case MultipleScatteringProbability:
        return m_simulationEngine->getMultipleScatteringProbability();
    case MaximumRaysPerFrame:
        return m_simulationEngine->getRaysPerStep();
    case MaximumIterations:
        return m_maximumIterations;
    default:
        break;
    }

    return QVariant();
}

bool SimulationStateModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole) return false;

    if (data(index, role) == value) return false;

    switch (index.column())
    {
    case SunAltitude:
        setSunAltitude(value.toFloat());
        break;
    case SunDiameter:
        setSunDiameter(value.toFloat());
        break;
    case CameraProjection:
        setCameraProjection(static_cast<Projection>(value.toInt()));
        break;
    case CameraFov:
        setCameraFov(value.toFloat());
        break;
    case CameraPitch:
        setCameraPitch(value.toFloat());
        break;
    case CameraYaw:
        setCameraYaw(value.toFloat());
        break;
    case HideSubHorizon:
        setHideSubHorizon(value.toBool());
        break;
    case MultipleScatteringProbability:
        m_simulationEngine->setMultipleScatteringProbability(value.toDouble());
        break;
    case MaximumRaysPerFrame:
        m_simulationEngine->setRaysPerStep(value.toUInt());
        break;
    case MaximumIterations:
        m_maximumIterations = value.toUInt();
        break;
    default:
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags SimulationStateModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == CameraMaxFov)
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

void SimulationStateModel::setMaxRaysPerFrame(unsigned int maxRaysPerFrame)
{
    setData(index(0, MaximumRaysPerFrame), maxRaysPerFrame);
}

unsigned int SimulationStateModel::getMaxIterations() const
{
    return m_maximumIterations;
}

void SimulationStateModel::setSunAltitude(float altitude)
{
    auto lightSource = m_simulationEngine->getLightSource();
    lightSource.altitude = altitude;
    m_simulationEngine->setLightSource(lightSource);
}

void SimulationStateModel::setSunDiameter(float diameter)
{
    auto lightSource = m_simulationEngine->getLightSource();
    lightSource.diameter = diameter;
    m_simulationEngine->setLightSource(lightSource);
}

void SimulationStateModel::setCameraProjection(Projection projection)
{
    auto camera = m_simulationEngine->getCamera();
    camera.projection = projection;
    m_simulationEngine->setCamera(camera);
}

void SimulationStateModel::setCameraFov(float fov)
{
    auto camera = m_simulationEngine->getCamera();
    camera.fov = fov;
    m_simulationEngine->setCamera(camera);
}

void SimulationStateModel::setCameraPitch(float pitch)
{
    auto camera = m_simulationEngine->getCamera();
    camera.pitch = pitch;
    m_simulationEngine->setCamera(camera);
}

void SimulationStateModel::setCameraYaw(float yaw)
{
    auto camera = m_simulationEngine->getCamera();
    camera.yaw = yaw;
    m_simulationEngine->setCamera(camera);
}

void SimulationStateModel::setHideSubHorizon(bool hide)
{
    auto camera = m_simulationEngine->getCamera();
    camera.hideSubHorizon = hide;
    m_simulationEngine->setCamera(camera);
}

}
