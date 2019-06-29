#include "mainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include "../simulation/simulationEngine.h"
#include "sliderSpinBox.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi();
    mEngine = std::make_shared<HaloSim::SimulationEngine>(mOpenGLWidget->width(), mOpenGLWidget->height());
    mOpenGLWidget->setEngine(mEngine);

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

    connect(mRenderButton, &QPushButton::clicked, mOpenGLWidget, &OpenGLWidget::toggleRendering);
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::lightSourceChanged, [=](HaloSim::LightSource light) {
        mEngine->SetLightSource(light);
    });
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::numRaysChanged, [=](unsigned int value) {
        mEngine->SetRaysPerStep(value);
    });
    connect(mCaRatioSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        auto crystals = mEngine->GetCrystalPopulation();
        crystals.caRatioAverage = value;
        mEngine->SetCrystalPopulation(crystals);
    });
    connect(mCaRatioStdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        auto crystals = mEngine->GetCrystalPopulation();
        crystals.caRatioStd = value;
        mEngine->SetCrystalPopulation(crystals);
    });
    connect(mCameraProjectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        auto camera = mEngine->GetCamera();
        camera.projection = (HaloSim::Projection)index;
        mEngine->SetCamera(camera);
    });

    mGeneralSettingsWidget->SetInitialValues(mEngine->GetLightSource().diameter,
                                             mEngine->GetLightSource().altitude,
                                             mEngine->GetRaysPerStep(),
                                             1000000);

    mCaRatioSpinBox->setValue(mEngine->GetCrystalPopulation().caRatioAverage);
    mCaRatioStdSpinBox->setValue(mEngine->GetCrystalPopulation().caRatioStd);
}

void MainWindow::setupUi()
{
    mGeneralSettingsWidget = new GeneralSettingsWidget();

    mOpenGLWidget = new OpenGLWidget();
    mOpenGLWidget->setMinimumSize(800, 800);

    mCaRatioSpinBox = new QDoubleSpinBox();

    mCaRatioStdSpinBox = new QDoubleSpinBox();

    mRenderButton = new QPushButton("Render / Stop");
    mRenderButton->setMinimumHeight(100);

    mCameraProjectionComboBox = new QComboBox();
    mCameraProjectionComboBox->addItem("Stereographic");
    mCameraProjectionComboBox->addItem("Rectilinear");
    mCameraProjectionComboBox->addItem("Equidistant");
    mCameraProjectionComboBox->addItem("Equal area");
    mCameraProjectionComboBox->addItem("Orthographic");

    auto mainWidget = new QWidget();
    auto topLayout = new QHBoxLayout();
    mainWidget->setLayout(topLayout);
    setCentralWidget(mainWidget);

    auto sideBarLayout = new QVBoxLayout();

    auto generalSettingsGroupBox = new QGroupBox("General settings");
    auto generalSettingsLayout = new QHBoxLayout();
    generalSettingsLayout->addWidget(mGeneralSettingsWidget);
    generalSettingsGroupBox->setLayout(generalSettingsLayout);

    auto crystalSettingsGroupBox = new QGroupBox("Crystal settings");
    auto crystalSettingsLayout = new QFormLayout();
    crystalSettingsLayout->addRow("C/A average ratio", mCaRatioSpinBox);
    crystalSettingsLayout->addRow("C/A ratio std.", mCaRatioStdSpinBox);
    crystalSettingsGroupBox->setLayout(crystalSettingsLayout);

    auto viewSettingsGroupBox = new QGroupBox("View settings");
    auto viewSettingsLayout = new QFormLayout();
    viewSettingsLayout->addRow("Camera projection", mCameraProjectionComboBox);
    viewSettingsGroupBox->setLayout(viewSettingsLayout);

    sideBarLayout->addWidget(generalSettingsGroupBox);
    sideBarLayout->addWidget(crystalSettingsGroupBox);
    sideBarLayout->addWidget(viewSettingsGroupBox);
    sideBarLayout->addWidget(mRenderButton);

    topLayout->addLayout(sideBarLayout);
    topLayout->addWidget(mOpenGLWidget);

    this->setWindowTitle("HaloRay");
    this->setMinimumSize(640, 480);
}
