#include "stateSaver.h"
#include <QtGlobal>
#include <QString>
#include <QSettings>
#include "gui/models/simulationStateModel.h"
#include "gui/models/crystalModel.h"
#include "simulation/simulationEngine.h"
#include "simulation/crystalPopulationRepository.h"

namespace HaloRay
{

void StateSaver::SaveState(QString filename, SimulationEngine *engine, CrystalPopulationRepository *crystals)
{
    qInfo("Saving simulation state to: %s", filename.toUtf8().constData());
    QSettings settings(filename, QSettings::Format::IniFormat);

    settings.beginGroup("LightSource");
    auto lightSource = engine->getLightSource();
    settings.setValue("Altitude", (double)lightSource.altitude);
    settings.setValue("Diameter", (double)lightSource.diameter);
    settings.endGroup();

    settings.beginGroup("CrystalPopulations");
    settings.beginWriteArray("pop");
    for (auto i = 0u; i < crystals->getCount(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Weight", crystals->getWeight(i));
        auto population = crystals->get(i);

        settings.setValue("Name", QString::fromStdString(crystals->getName(i)));
        settings.setValue("Enabled", population.enabled);

        settings.setValue("CaRatioAverage", (double)population.caRatioAverage);
        settings.setValue("CaRatioStd", (double)population.caRatioStd);

        settings.setValue("TiltDistribution", population.tiltDistribution);
        settings.setValue("TiltAverage", (double)population.tiltAverage);
        settings.setValue("TiltStd", (double)population.tiltStd);

        settings.setValue("RotationDistribution", population.rotationDistribution);
        settings.setValue("RotationAverage", (double)population.rotationAverage);
        settings.setValue("RotationStd", (double)population.rotationStd);

        settings.setValue("UpperApexAngle", (double)population.upperApexAngle);
        settings.setValue("UpperApexHeightAverage", (double)population.upperApexHeightAverage);
        settings.setValue("UpperApexHeightStd", (double)population.upperApexHeightStd);

        settings.setValue("LowerApexAngle", (double)population.lowerApexAngle);
        settings.setValue("LowerApexHeightAverage", (double)population.lowerApexHeightAverage);
        settings.setValue("LowerApexHeightStd", (double)population.lowerApexHeightStd);
    }
    settings.endArray();
    settings.endGroup();

    settings.beginGroup("Camera");
    auto camera = engine->getCamera();
    settings.setValue("Projection", (double)camera.projection);
    settings.setValue("Pitch", (double)camera.pitch);
    settings.setValue("Yaw", (double)camera.yaw);
    settings.setValue("FieldOfView", (double)camera.fov);
    settings.setValue("HideSubHorizon", camera.hideSubHorizon);
    settings.endGroup();

    settings.beginGroup("Atmosphere");
    auto atmosphere = engine->getAtmosphere();
    settings.setValue("Enabled", atmosphere.enabled);
    settings.setValue("Turbidity", atmosphere.turbidity);
    settings.setValue("GroundAlbedo", atmosphere.groundAlbedo);
    settings.endGroup();
    qInfo("Finished saving simulation state");
}

void StateSaver::LoadState(QString filename, SimulationStateModel *simState, CrystalModel *crystalModel)
{
    qInfo("Loading simulation state from: %s", filename.toUtf8().constData());
    QSettings settings(filename, QSettings::Format::IniFormat);

    auto lightSource = LightSource::createDefaultLightSource();
    lightSource.altitude = settings.value("LightSource/Altitude", lightSource.altitude).toFloat();
    lightSource.diameter = settings.value("LightSource/Diameter", lightSource.diameter).toFloat();
    simState->setLightSource(lightSource);

    auto camera = Camera::createDefaultCamera();
    camera.projection = (Projection)settings.value("Camera/Projection", camera.projection).toInt();
    camera.pitch = settings.value("Camera/Pitch", camera.pitch).toFloat();
    camera.yaw = settings.value("Camera/Yaw", camera.yaw).toFloat();
    camera.fov = settings.value("Camera/FieldOfView", camera.fov).toFloat();
    camera.hideSubHorizon = settings.value("Camera/HideSubHorizon", camera.hideSubHorizon).toBool();
    simState->setCamera(camera);

    crystalModel->clear();
    auto crystalPopulationCount = settings.beginReadArray("CrystalPopulations/pop");
    for (auto i = 0; i < crystalPopulationCount; ++i)
    {
        settings.setArrayIndex(i);
        auto pop = CrystalPopulation::createRandom();
        unsigned int weight = settings.value("Weight", 1).toUInt();
        pop.enabled = settings.value("Enabled", pop.enabled).toBool();

        pop.caRatioAverage = settings.value("CaRatioAverage", pop.caRatioAverage).toFloat();
        pop.caRatioStd = settings.value("CaRatioStd", pop.caRatioStd).toFloat();

        pop.tiltDistribution = settings.value("TiltDistribution", pop.tiltDistribution).toInt();
        pop.tiltAverage = settings.value("TiltAverage", pop.tiltAverage).toFloat();
        pop.tiltStd = settings.value("TiltStd", pop.tiltStd).toFloat();

        pop.rotationDistribution = settings.value("RotationDistribution", pop.rotationDistribution).toInt();
        pop.rotationAverage = settings.value("RotationAverage", pop.rotationAverage).toFloat();
        pop.rotationStd = settings.value("RotationStd", pop.rotationStd).toFloat();

        pop.upperApexAngle = settings.value("UpperApexAngle", pop.upperApexAngle).toFloat();
        pop.upperApexHeightAverage = settings.value("UpperApexHeightAverage", pop.upperApexHeightAverage).toFloat();
        pop.upperApexHeightStd = settings.value("UpperApexHeightStd", pop.upperApexHeightStd).toFloat();

        pop.lowerApexAngle = settings.value("LowerApexAngle", pop.lowerApexAngle).toFloat();
        pop.lowerApexHeightAverage = settings.value("LowerApexHeightAverage", pop.lowerApexHeightAverage).toFloat();
        pop.lowerApexHeightStd = settings.value("LowerApexHeightStd", pop.lowerApexHeightStd).toFloat();

        auto name = settings.value("Name", "Default name").toString();

        crystalModel->addRow(pop, weight, name);
    }
    settings.endArray();

    auto atmosphere = Atmosphere::createDefaultAtmosphere();
    atmosphere.enabled = settings.value("Atmosphere/Enabled", atmosphere.enabled).toBool();
    atmosphere.turbidity = settings.value("Atmosphere/Turbidity", atmosphere.turbidity).toDouble();
    atmosphere.groundAlbedo = settings.value("Atmosphere/GroundAlbedo", atmosphere.groundAlbedo).toDouble();
    simState->setAtmosphere(atmosphere);
    qInfo("Finished loading simulation state");
}


}
