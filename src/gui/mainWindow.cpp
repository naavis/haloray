#include "mainWindow.h"
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "openGLWidget.h"
#include "../simulation/simulationEngine.h"
#include "sliderSpinBox.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mEngine = std::make_shared<HaloSim::SimulationEngine>(ui->openGLWidget->width(), ui->openGLWidget->height());
    ui->openGLWidget->setEngine(mEngine);

    HaloSim::Camera camera;
    camera.pitch = 0.0f;
    camera.yaw = 0.0f;
    camera.fov = 0.5f;
    camera.projection = HaloSim::Projection::Stereographic;
    camera.hideSubHorizon = false;

    HaloSim::LightSource sun;
    sun.altitude = 30.0f;
    sun.diameter = 0.5f;

    HaloSim::CrystalPopulation crystalProperties;
    crystalProperties.caRatioAverage = 0.3f;
    crystalProperties.caRatioStd = 0.0f;

    crystalProperties.tiltDistribution = 1;
    crystalProperties.tiltAverage = 0.0f;
    crystalProperties.tiltStd = 40.0f;

    crystalProperties.rotationDistribution = 1;
    crystalProperties.rotationAverage = 0.0f;
    crystalProperties.rotationStd = 1.0f;

    mEngine->SetCamera(camera);
    mEngine->SetLightSource(sun);
    mEngine->SetCrystalPopulation(crystalProperties);

    connect(ui->renderButton, &QPushButton::clicked, ui->openGLWidget, &OpenGLWidget::toggleRendering);
    connect(ui->generalSettingsWidget, &GeneralSettingsWidget::lightSourceChanged, [=](HaloSim::LightSource light) {
        mEngine->SetLightSource(light);
    });
    connect(ui->generalSettingsWidget, &GeneralSettingsWidget::numRaysChanged, [=](unsigned int value) {
        mEngine->SetRaysPerStep(value);
    });
    connect(ui->caRatioSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        auto crystals = mEngine->GetCrystalPopulation();
        crystals.caRatioAverage = value;
        mEngine->SetCrystalPopulation(crystals);
    });
    connect(ui->caRatioStdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        auto crystals = mEngine->GetCrystalPopulation();
        crystals.caRatioStd = value;
        mEngine->SetCrystalPopulation(crystals);
    });
    connect(ui->cameraProjectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        auto camera = mEngine->GetCamera();
        camera.projection = (HaloSim::Projection)index;
        mEngine->SetCamera(camera);
    });

    ui->generalSettingsWidget->SetInitialValues(mEngine->GetLightSource().diameter,
                                                mEngine->GetLightSource().altitude,
                                                mEngine->GetRaysPerStep(),
                                                1000000);

    ui->caRatioSpinBox->setValue(mEngine->GetCrystalPopulation().caRatioAverage);
    ui->caRatioStdSpinBox->setValue(mEngine->GetCrystalPopulation().caRatioStd);
}
