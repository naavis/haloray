#include "simulationStateModel.h"
#include "simulation/atmosphere.h"
#include "simulation/camera.h"
#include "simulation/lightSource.h"
#include "simulation/simulationEngine.h"

namespace HaloRay
{

SimulationStateModel::SimulationStateModel(SimulationEngine *engine, QObject *parent)
    : QAbstractTableModel(parent),
      m_simulationEngine(engine),
      m_maximumIterations(600),
      m_raysPerFrameUpperLimit(50000000)
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

    connect(m_simulationEngine, &SimulationEngine::raysPerStepChanged, [this]() {
        emit dataChanged(createIndex(0, RaysPerFrame), createIndex(0, RaysPerFrame));
    });

    connect(m_simulationEngine, &SimulationEngine::atmosphereChanged, [this]() {
        emit dataChanged(createIndex(0, AtmosphereEnabled), createIndex(0, GroundAlbedo));
    });

    connect(m_simulationEngine, &SimulationEngine::guidesToggled, [this]() {
        emit dataChanged(createIndex(0, GuidesEnabled), createIndex(0, GuidesEnabled));
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
        case RaysPerFrame:
            return "Rays per frame";
        case MaximumIterations:
            return "Maximum iterations";
        case RaysPerFrameUpperLimit:
            return "Rays per frame upper limit";
        case AtmosphereEnabled:
            return "Atmosphere enabled";
        case Turbidity:
            return "Atmosphere turbidity";
        case GroundAlbedo:
            return "Ground albedo";
        case GuidesEnabled:
            return "Guides enabled";
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
    case RaysPerFrame:
        return m_simulationEngine->getRaysPerStep();
    case MaximumIterations:
        return m_maximumIterations;
    case RaysPerFrameUpperLimit:
        return m_raysPerFrameUpperLimit;
    case AtmosphereEnabled:
        return m_simulationEngine->getAtmosphere().enabled;
    case Turbidity:
        return m_simulationEngine->getAtmosphere().turbidity;
    case GroundAlbedo:
        return m_simulationEngine->getAtmosphere().groundAlbedo;
    case GuidesEnabled:
        return m_simulationEngine->getGuidesEnabled();
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
    case RaysPerFrame:
        m_simulationEngine->setRaysPerStep(value.toUInt());
        break;
    case MaximumIterations:
        m_maximumIterations = value.toUInt();
        break;
    case RaysPerFrameUpperLimit:
        m_raysPerFrameUpperLimit = value.toUInt();
        break;
    case AtmosphereEnabled:
        setAtmosphereEnabled(value.toBool());
        break;
    case Turbidity:
        setAtmosphereTurbidity(value.toDouble());
        break;
    case GroundAlbedo:
        setGroundAlbedo(value.toDouble());
        break;
    case GuidesEnabled:
        m_simulationEngine->setGuidesEnabled(value.toBool());
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

void SimulationStateModel::setRaysPerFrame(unsigned int maxRaysPerFrame)
{
    setData(index(0, RaysPerFrame), maxRaysPerFrame);
}

void SimulationStateModel::setRaysPerFrameUpperLimit(unsigned int upperLimit)
{
    setData(index(0, RaysPerFrameUpperLimit), upperLimit);
}

unsigned int SimulationStateModel::getRaysPerFrameUpperLimit() const
{
    return m_raysPerFrameUpperLimit;
}

unsigned int SimulationStateModel::getMaxIterations() const
{
    return m_maximumIterations;
}

void SimulationStateModel::setLightSource(LightSource lightSource)
{
    m_simulationEngine->setLightSource(lightSource);
    emit dataChanged(createIndex(0, SunAltitude), createIndex(0, SunDiameter));
}

void SimulationStateModel::setCamera(Camera camera)
{
    m_simulationEngine->setCamera(camera);
    emit dataChanged(createIndex(0, CameraProjection), createIndex(0, HideSubHorizon));
}

void SimulationStateModel::setAtmosphere(Atmosphere atmosphere)
{
    m_simulationEngine->setAtmosphere(atmosphere);
    emit dataChanged(createIndex(0, AtmosphereEnabled), createIndex(0, GroundAlbedo));
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

void SimulationStateModel::setAtmosphereEnabled(bool enabled)
{
    auto atmosphere = m_simulationEngine->getAtmosphere();
    atmosphere.enabled = enabled;
    m_simulationEngine->setAtmosphere(atmosphere);
}

void SimulationStateModel::setAtmosphereTurbidity(float turbidity)
{
    auto atmosphere = m_simulationEngine->getAtmosphere();
    atmosphere.turbidity = turbidity;
    m_simulationEngine->setAtmosphere(atmosphere);
}

void SimulationStateModel::setGroundAlbedo(float albedo)
{
    auto atmosphere = m_simulationEngine->getAtmosphere();
    atmosphere.groundAlbedo = albedo;
    m_simulationEngine->setAtmosphere(atmosphere);
}

}
