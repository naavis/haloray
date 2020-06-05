#pragma once
#include <QMainWindow>
#include <memory>


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

    GeneralSettingsWidget *mGeneralSettingsWidget;
    CrystalSettingsWidget *mCrystalSettingsWidget;
    ViewSettingsWidget *mViewSettingsWidget;
    QProgressBar *mProgressBar;
    RenderButton *mRenderButton;
    OpenGLWidget *mOpenGLWidget;

    QAction *mSaveImageAction;
    QAction *mQuitAction;

    std::shared_ptr<HaloRay::CrystalPopulationRepository> mCrystalRepository;
    std::shared_ptr<HaloRay::SimulationEngine> mEngine;
};

}
