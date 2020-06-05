#pragma once
#include <QMainWindow>
#include <memory>

namespace HaloSim
{
class CrystalPopulationRepository;
class SimulationEngine;
}

class QDoubleSpinBox;
class QProgressBar;
class QScrollArea;
class QAction;
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

    GeneralSettingsWidget *mGeneralSettingsWidget;
    CrystalSettingsWidget *mCrystalSettingsWidget;
    ViewSettingsWidget *mViewSettingsWidget;
    QProgressBar *mProgressBar;
    RenderButton *mRenderButton;
    OpenGLWidget *mOpenGLWidget;

    QAction *mSaveImageAction;
    QAction *mQuitAction;

    std::shared_ptr<HaloSim::CrystalPopulationRepository> mCrystalRepository;
    std::shared_ptr<HaloSim::SimulationEngine> mEngine;
};
