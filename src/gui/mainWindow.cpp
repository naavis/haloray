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
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::lightSourceChanged, [=](HaloSim::LightSource light) {
        mEngine->SetLightSource(light);
    });
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::numRaysChanged, [=](unsigned int value) {
        mEngine->SetRaysPerStep(value);
    });

    connect(mCrystalSettingsWidget, &CrystalSettingsWidget::crystalChanged, [=](HaloSim::CrystalPopulation crystal) {
        mEngine->SetCrystalPopulation(crystal);
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

    mCrystalSettingsWidget->SetInitialValues(mEngine->GetCrystalPopulation());
}

void MainWindow::setupUi()
{
    mGeneralSettingsWidget = new GeneralSettingsWidget();

    mCrystalSettingsWidget = new CrystalSettingsWidget();

    mOpenGLWidget = new OpenGLWidget();
    mOpenGLWidget->setMinimumSize(800, 800);

    mRenderButton = new QPushButton("Render / Stop");
    mRenderButton->setMinimumHeight(100);

    mCameraProjectionComboBox = new QComboBox();
    mCameraProjectionComboBox->addItems({"Stereographic",
                                         "Rectilinear",
                                         "Equidistant",
                                         "Equal area",
                                         "Orthographic"});

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
    auto crystalSettingsLayout = new QHBoxLayout();
    crystalSettingsLayout->addWidget(mCrystalSettingsWidget);
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
