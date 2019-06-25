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

    connect(ui->renderButton, &QPushButton::clicked, ui->openGLWidget, &OpenGLWidget::toggleRendering);
    connect(ui->generalSettingsWidget, &GeneralSettingsWidget::lightSourceChanged, [=](HaloSim::LightSource light) {
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
