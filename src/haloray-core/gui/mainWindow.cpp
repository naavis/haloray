#include "mainWindow.h"
#include "simulationStateViewModel.h"
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
#include "openGLWidget.h"
#include "generalSettingsWidget.h"
#include "crystalSettingsWidget.h"
#include "viewSettingsWidget.h"
#include "sliderSpinBox.h"
#include "renderButton.h"
#include "../simulation/crystalPopulation.h"
#include "../simulation/simulationEngine.h"

#define STRINGIFY0(v) #v
#define STRINGIFY(v) STRINGIFY0(v)

namespace HaloRay
{

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
#if _WIN32
    QIcon::setThemeName("HaloRayTheme");
#endif
    m_crystalRepository = std::make_shared<HaloRay::CrystalPopulationRepository>();
    m_engine = new SimulationEngine(m_crystalRepository, this);

    m_simulationStateViewModel = new SimulationStateViewModel(m_engine, this);

    setupUi();

    // Signals from render button
    connect(m_renderButton, &RenderButton::clicked, m_openGLWidget, &OpenGLWidget::toggleRendering);
    connect(m_renderButton, &RenderButton::clicked, m_generalSettingsWidget, &GeneralSettingsWidget::toggleMaxIterationsSpinBoxStatus);

    // Signals from crystal settings
    connect(m_crystalSettingsWidget, &CrystalSettingsWidget::crystalChanged, [this]() {
        m_engine->clear();
        m_openGLWidget->update();
    });

    // Signals from view settings
    connect(m_viewSettingsWidget, &ViewSettingsWidget::brightnessChanged, m_openGLWidget, &OpenGLWidget::setBrightness);
    connect(m_viewSettingsWidget, &ViewSettingsWidget::lockToLightSource, [this](bool locked) {
        m_engine->lockCameraToLightSource(locked);
        m_openGLWidget->update();
    });
    m_viewSettingsWidget->setBrightness(1.0);

    // Signals from OpenGL widget
    connect(m_openGLWidget, &OpenGLWidget::maxRaysPerFrameChanged, m_generalSettingsWidget, &GeneralSettingsWidget::setMaxRaysPerFrame);
    connect(m_openGLWidget, &OpenGLWidget::nextIteration, m_progressBar, &QProgressBar::setValue);

    connect(m_generalSettingsWidget, &GeneralSettingsWidget::numRaysChanged, [this](unsigned int value) {
        m_engine->setRaysPerStep(value);
        m_openGLWidget->update();
    });
    connect(m_generalSettingsWidget, &GeneralSettingsWidget::maximumNumberOfIterationsChanged, m_openGLWidget, &OpenGLWidget::setMaxIterations);
    connect(m_generalSettingsWidget, &GeneralSettingsWidget::maximumNumberOfIterationsChanged, m_progressBar, &QProgressBar::setMaximum);

    m_generalSettingsWidget->setInitialValues(m_engine->getRaysPerStep(), 600);

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

    setupMenuBar();

    m_openGLWidget = new OpenGLWidget(m_engine);
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
    m_quitAction = fileMenu->addAction(tr("&Quit"));
}

QScrollArea *MainWindow::setupSideBarScrollArea()
{
    m_generalSettingsWidget = new GeneralSettingsWidget(m_simulationStateViewModel);
    m_crystalSettingsWidget = new CrystalSettingsWidget(m_crystalRepository);
    m_viewSettingsWidget = new ViewSettingsWidget(m_simulationStateViewModel);

    auto scrollContainer = new QWidget();
    auto scrollableLayout = new QVBoxLayout(scrollContainer);
    scrollableLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    scrollableLayout->addWidget(m_generalSettingsWidget);
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
    return progressBar;
}

QSize MainWindow::sizeHint() const
{
    return QSize(1920, 1080);
}

}
