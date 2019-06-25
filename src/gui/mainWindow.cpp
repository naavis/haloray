#include "mainWindow.h"
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "openGLWidget.h"
#include "../simulation/simulationEngine.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mEngine = std::make_shared<HaloSim::SimulationEngine>(ui->openGLWidget->width(), ui->openGLWidget->height());
    ui->openGLWidget->setEngine(mEngine);

    connect(ui->renderButton, &QPushButton::clicked, ui->openGLWidget, &OpenGLWidget::toggleRendering);
    connect(ui->sunAltitudeSlider, &QSlider::valueChanged, [=](int value) {
        auto light = mEngine->GetLightSource();
        light.altitude = (float)value;
        mEngine->SetLightSource(light);
    });
    connect(ui->sunDiameterSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        auto light = mEngine->GetLightSource();
        light.diameter = value;
        mEngine->SetLightSource(light);
    });
    connect(ui->caRatioSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        auto crystals = mEngine->GetCrystalPopulation();
        crystals.caRatioAverage = value;
        mEngine->SetCrystalPopulation(crystals);
    });
    connect(ui->caRatioStdSpinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        auto crystals = mEngine->GetCrystalPopulation();
        crystals.caRatioStd = value;
        mEngine->SetCrystalPopulation(crystals);
    });
    connect(ui->cameraProjectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        auto camera = mEngine->GetCamera();
        camera.projection = (HaloSim::Projection)index;
        mEngine->SetCamera(camera);
    });
}
