#pragma once
#include <QMainWindow>
#include <memory>
#include "simulationStateViewModel.h"


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

    GeneralSettingsWidget *m_generalSettingsWidget;
    CrystalSettingsWidget *m_crystalSettingsWidget;
    ViewSettingsWidget *m_viewSettingsWidget;
    QProgressBar *m_progressBar;
    RenderButton *m_renderButton;
    OpenGLWidget *m_openGLWidget;

    QAction *m_saveImageAction;
    QAction *m_quitAction;

    std::shared_ptr<CrystalPopulationRepository> m_crystalRepository;
    SimulationEngine *m_engine;

    SimulationStateViewModel *m_simulationStateViewModel;
};

}
