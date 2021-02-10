#include "mainWindow.h"
#include "simulationStateModel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QString>
#include <QScrollBar>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>
#include <QFileDialog>
#include <QDateTime>
#include <QProgressBar>
#include <QScrollArea>
#include <QStatusBar>
#include <QSettings>
#include "../simulation/atmosphere.h"
#include "crystalModel.h"
#include "openGLWidget.h"
#include "generalSettingsWidget.h"
#include "crystalSettingsWidget.h"
#include "viewSettingsWidget.h"
#include "atmosphereSettingsWidget.h"
#include "sliderSpinBox.h"
#include "renderButton.h"
#include "../simulation/crystalPopulation.h"
#include "../simulation/simulationEngine.h"

#define STRINGIFY0(v) #v
#define STRINGIFY(v) STRINGIFY0(v)

namespace HaloRay
{

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_previousTimedIteration(0)
{
#if _WIN32
    QIcon::setThemeName("HaloRayTheme");
#endif

    m_crystalRepository = std::make_shared<CrystalPopulationRepository>();
    m_engine = new SimulationEngine(m_crystalRepository, this);

    m_crystalModel = new CrystalModel(m_crystalRepository, this);
    m_simulationStateModel = new SimulationStateModel(m_engine, this);

    setupUi();

    // Signals from render button
    connect(m_renderButton, &RenderButton::clicked, m_openGLWidget, &OpenGLWidget::toggleRendering);
    connect(m_renderButton, &RenderButton::clicked, m_generalSettingsWidget, &GeneralSettingsWidget::toggleComputeShaderParametersEnabled);

    // Signals from crystal model
    connect(m_crystalModel, &CrystalModel::dataChanged, [this]() {
        m_engine->clear();
        m_openGLWidget->update();
    });

    // Signals from view settings
    connect(m_viewSettingsWidget, &ViewSettingsWidget::brightnessChanged, m_openGLWidget, &OpenGLWidget::setBrightness);
    connect(m_viewSettingsWidget, &ViewSettingsWidget::lockToLightSource, [this](bool locked) {
        m_engine->lockCameraToLightSource(locked);
        m_openGLWidget->update();
    });
    m_viewSettingsWidget->setBrightness(3.0);

    // Signals from OpenGL widget
    connect(m_openGLWidget, &OpenGLWidget::nextIteration, m_progressBar, &QProgressBar::setValue);

    // Signals from view model
    connect(m_simulationStateModel, &SimulationStateModel::dataChanged, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        if (topLeft.row() != 0 || bottomRight.row() != 0) return;
        auto maxIterationsChanged = topLeft.column() <= SimulationStateModel::MaximumIterations && bottomRight.column() >= SimulationStateModel::MaximumIterations;
        if (maxIterationsChanged) {
            m_progressBar->setMaximum(m_simulationStateModel->getMaxIterations());
        }
    });

    // Signals for menu bar
    connect(m_quitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(m_saveImageAction, &QAction::triggered, [this]() {
        auto image = m_openGLWidget->grabFramebuffer();
        auto currentTime = QDateTime::currentDateTimeUtc().toString(Qt::DateFormat::ISODate);
        auto defaultFilename = QString("haloray_%1.png")
                                   .arg(currentTime)
                                   .replace(":", "-");
        QString filename = QFileDialog::getSaveFileName(this,
                                                        tr("Save File"),
                                                        defaultFilename,
                                                        tr("Images (*.png)"));

        if (!filename.isNull())
        {
            image.save(filename, "PNG", 50);
        }
    });
    connect(m_saveSimulationAction, &QAction::triggered, [this]() {
        auto currentTime = QDateTime::currentDateTimeUtc().toString(Qt::DateFormat::ISODate);
        auto defaultFilename = QString("haloray_sim_%1.ini")
                                   .arg(currentTime)
                                   .replace(":", "-");
        QString filename = QFileDialog::getSaveFileName(this,
                                                        tr("Save File"),
                                                        defaultFilename,
                                                        tr("Simulation files (*.ini)"));

        if (filename.isNull()) return;

        QSettings settings(filename, QSettings::Format::IniFormat);

        settings.beginGroup("LightSource");
        auto lightSource = m_engine->getLightSource();
        settings.setValue("Altitude", (double)lightSource.altitude);
        settings.setValue("Diameter", (double)lightSource.diameter);
        settings.endGroup();

        settings.beginGroup("CrystalPopulations");
        settings.beginWriteArray("pop");
        for (auto i = 0u; i < m_crystalRepository->getCount(); ++i)
        {
            settings.setArrayIndex(i);
            settings.setValue("Weight", m_crystalRepository->getWeight(i));
            auto population = m_crystalRepository->get(i);

            settings.setValue("Name", QString::fromStdString(m_crystalRepository->getName(i)));

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
        auto camera = m_engine->getCamera();
        settings.setValue("Projection", (double)camera.projection);
        settings.setValue("Pitch", (double)camera.pitch);
        settings.setValue("Yaw", (double)camera.yaw);
        settings.setValue("FieldOfView", (double)camera.fov);
        settings.setValue("HideSubHorizon", camera.hideSubHorizon);
        settings.endGroup();

        settings.beginGroup("Atmosphere");
        auto atmosphere = m_engine->getAtmosphere();
        settings.setValue("Enabled", atmosphere.enabled);
        settings.setValue("Turbidity", atmosphere.turbidity);
        settings.setValue("GroundAlbedo", atmosphere.groundAlbedo);
        settings.endGroup();
    });
    connect(m_loadSimulationAction, &QAction::triggered, [this]() {
        QString filename = QFileDialog::getOpenFileName(this,
                                                        tr("Open file"),
                                                        QString(),
                                                        tr("Simulation files (*.ini)"));

        if (filename.isNull()) return;


        QSettings settings(filename, QSettings::Format::IniFormat);

        auto lightSource = LightSource::createDefaultLightSource();
        lightSource.altitude = settings.value("LightSource/Altitude", lightSource.altitude).toFloat();
        lightSource.diameter = settings.value("LightSource/Diameter", lightSource.diameter).toFloat();
        m_simulationStateModel->setLightSource(lightSource);

        auto camera = Camera::createDefaultCamera();
        camera.projection = (Projection)settings.value("Camera/Projection", camera.projection).toInt();
        camera.pitch = settings.value("Camera/Pitch", camera.pitch).toFloat();
        camera.yaw = settings.value("Camera/Yaw", camera.yaw).toFloat();
        camera.fov = settings.value("Camera/FieldOfView", camera.fov).toFloat();
        camera.hideSubHorizon = settings.value("Camera/HideSubHorizon", camera.hideSubHorizon).toBool();
        m_simulationStateModel->setCamera(camera);

        m_crystalModel->clear();
        auto crystalPopulationCount = settings.beginReadArray("CrystalPopulations/pop");
        for (auto i = 0; i < crystalPopulationCount; ++i)
        {
            settings.setArrayIndex(i);
            auto pop = CrystalPopulation::createRandom();
            unsigned int weight = settings.value("Weight", 1).toUInt();

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

            m_crystalModel->addRow(pop, weight, name);
        }
        settings.endArray();

        auto atmosphere = Atmosphere::createDefaultAtmosphere();
        atmosphere.enabled = settings.value("Atmosphere/Enabled", atmosphere.enabled).toBool();
        atmosphere.turbidity = settings.value("Atmosphere/Turbidity", atmosphere.turbidity).toDouble();
        atmosphere.groundAlbedo = settings.value("Atmosphere/GroundAlbedo", atmosphere.groundAlbedo).toDouble();
        m_engine->setAtmosphere(atmosphere);
    });

    setupRenderTimer();
}

void MainWindow::setupUi()
{
#ifdef HALORAY_VERSION
    setWindowTitle(QString("HaloRay %1").arg(STRINGIFY(HALORAY_VERSION)));
#else
    setWindowTitle(QString("HaloRay - %1 - %2")
                       .arg(STRINGIFY(GIT_BRANCH))
                       .arg(STRINGIFY(GIT_COMMIT_HASH)));
#endif

    setWindowIcon(QIcon(":/haloray.ico"));

    this->statusBar()->show();
    setupMenuBar();

    m_openGLWidget = new OpenGLWidget(m_engine, m_simulationStateModel);
    m_progressBar = setupProgressBar();
    m_renderButton = new RenderButton();

    auto mainWidget = new QWidget();
    auto topLayout = new QHBoxLayout(mainWidget);
    setCentralWidget(mainWidget);

    auto sideBarLayout = new QVBoxLayout();
    auto sideBarScrollArea = setupSideBarScrollArea();
    sideBarLayout->addWidget(sideBarScrollArea);
    sideBarLayout->addWidget(m_progressBar);
    sideBarLayout->addWidget(m_renderButton);

    topLayout->addLayout(sideBarLayout);
    topLayout->addWidget(m_openGLWidget);
}

void MainWindow::setupMenuBar()
{
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    m_saveImageAction = fileMenu->addAction(tr("Save image"));
    fileMenu->addSeparator();
    m_loadSimulationAction = fileMenu->addAction(tr("Load simulation"));
    m_saveSimulationAction = fileMenu->addAction(tr("Save simulation"));
    fileMenu->addSeparator();
    m_quitAction = fileMenu->addAction(tr("&Quit"));
}

QScrollArea *MainWindow::setupSideBarScrollArea()
{
    m_generalSettingsWidget = new GeneralSettingsWidget(m_simulationStateModel);
    m_crystalSettingsWidget = new CrystalSettingsWidget(m_crystalModel);
    m_viewSettingsWidget = new ViewSettingsWidget(m_simulationStateModel);
    m_atmosphereSettingsWidget = new AtmosphereSettingsWidget(m_simulationStateModel);

    auto scrollContainer = new QWidget();
    auto scrollableLayout = new QVBoxLayout(scrollContainer);
    scrollableLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    scrollableLayout->addWidget(m_generalSettingsWidget);
    scrollableLayout->addWidget(m_atmosphereSettingsWidget);
    scrollableLayout->addWidget(m_crystalSettingsWidget);
    scrollableLayout->addWidget(m_viewSettingsWidget);
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
    progressBar->setMaximum(m_simulationStateModel->getMaxIterations());
    return progressBar;
}

QSize MainWindow::sizeHint() const
{
    return QSize(1920, 1080);
}

void MainWindow::setupRenderTimer()
{
    m_renderTimer.callOnTimeout([this]() {
        if (!m_engine->isRunning()) return;

        unsigned int currentIteration = m_engine->getIteration();
        unsigned int previousIteration = m_previousTimedIteration;
        if (currentIteration < previousIteration)
        {
            m_previousTimedIteration = currentIteration;
            return;
        }
        unsigned int raysPerStep = m_engine->getRaysPerStep();
        unsigned int rate = (currentIteration - previousIteration) * raysPerStep;
        m_previousTimedIteration = currentIteration;
        this->statusBar()->showMessage(QString("Simulation rate: %1 rays/s").arg(QLocale::system().toString(rate)));
    });

    connect(this->m_renderButton, &RenderButton::clicked, [this]() {
        if (m_engine->isRunning())
        {
            m_previousTimedIteration = 0;
            m_renderTimer.start(1000);
        }
        else
        {
            m_renderTimer.stop();
        }
    });
}

}
