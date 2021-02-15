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
#include "crystalPreview/crystalPreviewWindow.h"
#include "collapsibleBox.h"
#include "stateSaver.h"
#include "crystalModel.h"
#include "openGLWidget.h"
#include "generalSettingsWidget.h"
#include "crystalSettingsWidget.h"
#include "viewSettingsWidget.h"
#include "atmosphereSettingsWidget.h"
#include "sliderSpinBox.h"
#include "renderButton.h"
#include "../simulation/atmosphere.h"
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
        restartSimulation();
    });
    connect(m_crystalModel, &CrystalModel::rowsInserted, [this]() {
        restartSimulation();
    });
    connect(m_crystalModel, &CrystalModel::rowsRemoved, [this]() {
        restartSimulation();
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

        StateSaver::SaveState(filename, m_engine, m_crystalRepository.get());
    });
    connect(m_loadSimulationAction, &QAction::triggered, [this]() {
        QString filename = QFileDialog::getOpenFileName(this,
                                                        tr("Open file"),
                                                        QString(),
                                                        tr("Simulation files (*.ini)"));

        if (filename.isNull()) return;

        StateSaver::LoadState(filename, m_simulationStateModel, m_crystalModel);
    });
    connect(m_openCrystalPreviewWindow, &QAction::triggered, [this]() {
        auto previewWindow = new CrystalPreviewWindow(m_crystalModel, m_crystalSettingsWidget->getCurrentPopulationIndex(), this);
        connect(m_crystalSettingsWidget, &CrystalSettingsWidget::populationSelectionChanged, previewWindow, &CrystalPreviewWindow::onMainWindowPopulationSelectionChange);
        previewWindow->show();
    });
    connect(m_resetSimulationAction, &QAction::triggered, [this]() {
        m_crystalModel->clear();
        m_crystalModel->addRow(CrystalPopulationPreset::Random);
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
    m_resetSimulationAction = fileMenu->addAction(tr("&New simulation"));
    fileMenu->addSeparator();
    m_saveImageAction = fileMenu->addAction(tr("Save &image"));
    fileMenu->addSeparator();
    m_loadSimulationAction = fileMenu->addAction(tr("&Load simulation"));
    m_saveSimulationAction = fileMenu->addAction(tr("&Save simulation"));
    fileMenu->addSeparator();
    m_quitAction = fileMenu->addAction(tr("&Quit"));

    auto miscMenu = menuBar()->addMenu(tr("&View"));
    m_openCrystalPreviewWindow = miscMenu->addAction(tr("Crystal &preview"));
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
    scrollableLayout->addWidget(m_crystalSettingsWidget);
    scrollableLayout->addWidget(m_viewSettingsWidget);
    scrollableLayout->addWidget(m_atmosphereSettingsWidget);
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

void HaloRay::MainWindow::restartSimulation()
{
    m_engine->clear();
    m_openGLWidget->update();
}

}
