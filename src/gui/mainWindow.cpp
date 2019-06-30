#include "mainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include "../simulation/simulationEngine.h"
#include "../simulation/crystalPopulation.h"
#include "sliderSpinBox.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi();
    mEngine = std::make_shared<HaloSim::SimulationEngine>(mOpenGLWidget->width(), mOpenGLWidget->height());
    mOpenGLWidget->setEngine(mEngine);

    connect(mRenderButton, &QPushButton::clicked, mOpenGLWidget, &OpenGLWidget::toggleRendering);
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::lightSourceChanged, [this](HaloSim::LightSource light) {
        mEngine->SetLightSource(light);
    });
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::numRaysChanged, [this](unsigned int value) {
        mEngine->SetRaysPerStep(value);
    });

    connect(mCrystalSettingsWidget, &CrystalSettingsWidget::crystalChanged, [this](HaloSim::CrystalPopulation crystal) {
        mEngine->SetCrystalPopulation(crystal);
    });

    connect(mViewSettingsWidget, &ViewSettingsWidget::cameraChanged, [this](HaloSim::Camera camera) {
        mEngine->SetCamera(camera);
    });

    connect(mOpenGLWidget, &OpenGLWidget::fieldOfViewChanged, mViewSettingsWidget, &ViewSettingsWidget::setFieldOfView);
    connect(mOpenGLWidget, &OpenGLWidget::cameraOrientationChanged, mViewSettingsWidget, &ViewSettingsWidget::setCameraOrientation);
    connect(mViewSettingsWidget, &ViewSettingsWidget::brightnessChanged, mOpenGLWidget, &OpenGLWidget::setBrightness);

    mGeneralSettingsWidget->SetInitialValues(mEngine->GetLightSource().diameter,
                                             mEngine->GetLightSource().altitude,
                                             mEngine->GetRaysPerStep(),
                                             1000000);

    mCrystalSettingsWidget->SetInitialValues(mEngine->GetCrystalPopulation());
    mViewSettingsWidget->setCamera(mEngine->GetCamera());
    mViewSettingsWidget->setBrightness(1.0);
}

void MainWindow::setupUi()
{
    mGeneralSettingsWidget = new GeneralSettingsWidget();

    mCrystalSettingsWidget = new CrystalSettingsWidget();

    mViewSettingsWidget = new ViewSettingsWidget();

    mOpenGLWidget = new OpenGLWidget();
    mOpenGLWidget->setMinimumSize(800, 800);

    mRenderButton = new QPushButton("Render / Stop");
    mRenderButton->setMinimumHeight(100);

    auto mainWidget = new QWidget();
    auto topLayout = new QHBoxLayout();
    mainWidget->setLayout(topLayout);
    setCentralWidget(mainWidget);
    setLayout(topLayout);

    auto sideBarLayout = new QVBoxLayout();

    sideBarLayout->addWidget(mGeneralSettingsWidget);
    sideBarLayout->addWidget(mCrystalSettingsWidget);
    sideBarLayout->addWidget(mViewSettingsWidget);
    sideBarLayout->addWidget(mRenderButton);

    topLayout->addLayout(sideBarLayout);
    topLayout->addWidget(mOpenGLWidget);

    this->setWindowTitle("HaloRay");
}
