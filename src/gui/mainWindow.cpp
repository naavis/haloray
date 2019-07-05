#include "mainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QString>
#include <QScrollBar>
#include "../simulation/simulationEngine.h"
#include "../simulation/crystalPopulation.h"
#include "sliderSpinBox.h"
#include "../version.h"

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
    connect(mOpenGLWidget, &OpenGLWidget::maxRaysPerFrameChanged, mGeneralSettingsWidget, &GeneralSettingsWidget::setMaxRaysPerFrame);

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
    mOpenGLWidget = new OpenGLWidget();
    mProgressBar = setupProgressBar();
    mRenderButton = new RenderButton();

    auto mainWidget = new QWidget();
    auto topLayout = new QHBoxLayout(mainWidget);
    setCentralWidget(mainWidget);

    auto sideBarLayout = new QVBoxLayout();
    auto sideBarScrollArea = setupSideBarScrollArea();
    sideBarLayout->addWidget(sideBarScrollArea);
    sideBarLayout->addWidget(mProgressBar);
    sideBarLayout->addWidget(mRenderButton);

    topLayout->addLayout(sideBarLayout);
    topLayout->addWidget(mOpenGLWidget);

    setWindowTitle(QString("HaloRay %1").arg(HaloRay_VERSION));
}

QScrollArea *MainWindow::setupSideBarScrollArea()
{
    mGeneralSettingsWidget = new GeneralSettingsWidget();
    mCrystalSettingsWidget = new CrystalSettingsWidget();
    mViewSettingsWidget = new ViewSettingsWidget();

    auto scrollContainer = new QWidget();
    auto scrollableLayout = new QVBoxLayout(scrollContainer);
    scrollableLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    scrollableLayout->addWidget(mGeneralSettingsWidget);
    scrollableLayout->addWidget(mCrystalSettingsWidget);
    scrollableLayout->addWidget(mViewSettingsWidget);
    scrollableLayout->addStretch();

    auto scrollArea = new QScrollArea();
    scrollArea->setWidget(scrollContainer);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);

    scrollArea->setMinimumWidth(scrollContainer->minimumSize().width() + scrollArea->verticalScrollBar()->sizeHint().width());
    scrollArea->setMaximumWidth(scrollContainer->maximumSize().width() + scrollArea->verticalScrollBar()->sizeHint().width());

    return scrollArea;
}

QProgressBar *MainWindow::setupProgressBar()
{
    auto progressBar = new QProgressBar();
    progressBar->setTextVisible(false);
    progressBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    return progressBar;
}
