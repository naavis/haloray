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

    connect(mRenderButton, &RenderButton::clicked, mOpenGLWidget, &OpenGLWidget::toggleRendering);
    connect(mRenderButton, &RenderButton::clicked, mGeneralSettingsWidget, &GeneralSettingsWidget::toggleMaxIterationsSpinBoxStatus);

    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::lightSourceChanged, [this](HaloSim::LightSource light) {
        mEngine->SetLightSource(light);
    });
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::numRaysChanged, [this](unsigned int value) {
        mEngine->SetRaysPerStep(value);
    });
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::maximumNumberOfIterationsChanged, mOpenGLWidget, &OpenGLWidget::setMaxIterations);

    connect(mCrystalSettingsWidget, &CrystalSettingsWidget::crystalChanged, [this](HaloSim::CrystalPopulation crystal) {
        mEngine->SetCrystalPopulation(crystal);
    });

    connect(mViewSettingsWidget, &ViewSettingsWidget::cameraChanged, [this](HaloSim::Camera camera) {
        mEngine->SetCamera(camera);
    });
    connect(mViewSettingsWidget, &ViewSettingsWidget::brightnessChanged, mOpenGLWidget, &OpenGLWidget::setBrightness);
    connect(mViewSettingsWidget, &ViewSettingsWidget::lockToLightSource, [this](bool locked) {
        mEngine->LockCameraToLightSource(locked);
    });

    connect(mOpenGLWidget, &OpenGLWidget::fieldOfViewChanged, mViewSettingsWidget, &ViewSettingsWidget::setFieldOfView);
    connect(mOpenGLWidget, &OpenGLWidget::cameraOrientationChanged, mViewSettingsWidget, &ViewSettingsWidget::setCameraOrientation);

    connect(mOpenGLWidget, &OpenGLWidget::nextIteration, mProgressBar, &QProgressBar::setValue);
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::maximumNumberOfIterationsChanged, mProgressBar, &QProgressBar::setMaximum);

    mGeneralSettingsWidget->SetInitialValues(mEngine->GetLightSource().diameter,
                                             mEngine->GetLightSource().altitude,
                                             mEngine->GetRaysPerStep(),
                                             600);

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

    mProgressBar = new QProgressBar();
    mProgressBar->setTextVisible(false);
    mProgressBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    mRenderButton = new RenderButton();
    mRenderButton->setMinimumHeight(100);

    auto mainWidget = new QWidget();
    auto topLayout = new QHBoxLayout();
    mainWidget->setLayout(topLayout);
    setCentralWidget(mainWidget);

    auto sideBarLayout = new QVBoxLayout();

    sideBarLayout->addWidget(mGeneralSettingsWidget);
    sideBarLayout->addWidget(mCrystalSettingsWidget);
    sideBarLayout->addWidget(mViewSettingsWidget);
    sideBarLayout->addWidget(mProgressBar);
    sideBarLayout->addWidget(mRenderButton);

    topLayout->addLayout(sideBarLayout);
    topLayout->addWidget(mOpenGLWidget);

    setWindowTitle("HaloRay");
}
