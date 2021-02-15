#pragma once
#include <QMainWindow>
#include <memory>
#include <QTimer>
#include "simulationStateModel.h"


class QDoubleSpinBox;
class QProgressBar;
class QScrollArea;
class QAction;

namespace HaloRay
{

class CrystalPopulationRepository;
class SimulationEngine;
class OpenGLWidget;
class RenderButton;
class ViewSettingsWidget;
class GeneralSettingsWidget;
class CrystalSettingsWidget;
class AtmosphereSettingsWidget;
class CrystalModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    QSize sizeHint() const override;

private:
    void setupUi();
    QScrollArea *setupSideBarScrollArea();
    QProgressBar *setupProgressBar();
    void setupMenuBar();
    void setupRenderTimer();
    void restartSimulation();

    GeneralSettingsWidget *m_generalSettingsWidget;
    CrystalSettingsWidget *m_crystalSettingsWidget;
    ViewSettingsWidget *m_viewSettingsWidget;
    AtmosphereSettingsWidget *m_atmosphereSettingsWidget;
    QProgressBar *m_progressBar;
    RenderButton *m_renderButton;
    OpenGLWidget *m_openGLWidget;

    QAction *m_resetSimulationAction;
    QAction *m_saveImageAction;
    QAction *m_quitAction;
    QAction *m_saveSimulationAction;
    QAction *m_loadSimulationAction;
    QAction *m_openCrystalPreviewWindow;

    std::shared_ptr<CrystalPopulationRepository> m_crystalRepository;
    SimulationEngine *m_engine;

    SimulationStateModel *m_simulationStateModel;
    CrystalModel *m_crystalModel;
    QTimer m_renderTimer;
    int m_previousTimedIteration;
};

}
