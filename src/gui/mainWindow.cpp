#include "mainWindow.h"
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
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
}
