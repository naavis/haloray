#include "mainWindow.h"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
#if _WIN32
    QIcon::setThemeName("HaloRayTheme");
#endif

    mCrystalRepository = std::make_shared<HaloSim::CrystalPopulationRepository>();
    mEngine = std::make_shared<HaloSim::SimulationEngine>(mCrystalRepository);

    setupUi();

    // Signals from render button
    connect(mRenderButton, &RenderButton::clicked, mOpenGLWidget, &OpenGLWidget::toggleRendering);
    connect(mRenderButton, &RenderButton::clicked, mGeneralSettingsWidget, &GeneralSettingsWidget::toggleMaxIterationsSpinBoxStatus);

    // Signals from crystal settings
    connect(mCrystalSettingsWidget, &CrystalSettingsWidget::crystalChanged, [this]() {
        mEngine->clear();
        mOpenGLWidget->update();
    });

    // Signals from view settings
    connect(mViewSettingsWidget, &ViewSettingsWidget::cameraChanged, [this](HaloSim::Camera camera) {
        mEngine->setCamera(camera);
        mOpenGLWidget->update();
    });
    connect(mViewSettingsWidget, &ViewSettingsWidget::brightnessChanged, mOpenGLWidget, &OpenGLWidget::setBrightness);
    connect(mViewSettingsWidget, &ViewSettingsWidget::lockToLightSource, [this](bool locked) {
        mEngine->lockCameraToLightSource(locked);
        mOpenGLWidget->update();
    });
    mViewSettingsWidget->setCamera(mEngine->getCamera());
    mViewSettingsWidget->setBrightness(1.0);

    // Signals from OpenGL widget
    connect(mOpenGLWidget, &OpenGLWidget::fieldOfViewChanged, mViewSettingsWidget, &ViewSettingsWidget::setFieldOfView);
    connect(mOpenGLWidget, &OpenGLWidget::cameraOrientationChanged, mViewSettingsWidget, &ViewSettingsWidget::setCameraOrientation);
    connect(mOpenGLWidget, &OpenGLWidget::maxRaysPerFrameChanged, mGeneralSettingsWidget, &GeneralSettingsWidget::setMaxRaysPerFrame);
    connect(mOpenGLWidget, &OpenGLWidget::nextIteration, mProgressBar, &QProgressBar::setValue);

    // Signals from general settings
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::lightSourceChanged, [this](HaloSim::LightSource light) {
        mEngine->setLightSource(light);
        mOpenGLWidget->update();
    });
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::numRaysChanged, [this](unsigned int value) {
        mEngine->setRaysPerStep(value);
        mOpenGLWidget->update();
    });
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::maximumNumberOfIterationsChanged, mOpenGLWidget, &OpenGLWidget::setMaxIterations);
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::maximumNumberOfIterationsChanged, mProgressBar, &QProgressBar::setMaximum);
    connect(mGeneralSettingsWidget, &GeneralSettingsWidget::multipleScatteringProbabilityChanged, [this](double value) {
        mEngine->setMultipleScatteringProbability(value);
        mOpenGLWidget->update();
    });

    mGeneralSettingsWidget->setInitialValues(mEngine->getLightSource().diameter,
                                             mEngine->getLightSource().altitude,
                                             mEngine->getRaysPerStep(),
                                             600,
                                             mEngine->getMultipleScatteringProbability());

    // Signals for menu bar
    connect(mQuitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);
    connect(mSaveImageAction, &QAction::triggered, [this]() {
        auto image = mOpenGLWidget->grabFramebuffer();
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

    mOpenGLWidget = new OpenGLWidget(mEngine);
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
}

void MainWindow::setupMenuBar()
{
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    mSaveImageAction = fileMenu->addAction(tr("Save image"));
    fileMenu->addSeparator();
    mQuitAction = fileMenu->addAction(tr("&Quit"));
}

QScrollArea *MainWindow::setupSideBarScrollArea()
{
    mGeneralSettingsWidget = new GeneralSettingsWidget();
    mCrystalSettingsWidget = new CrystalSettingsWidget(mCrystalRepository);
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

QSize MainWindow::sizeHint() const
{
    return QSize(1920, 1080);
}
