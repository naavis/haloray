#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QScrollArea>
#include <memory>
#include "renderButton.h"
#include "openGLWidget.h"
#include "generalSettingsWidget.h"
#include "crystalSettingsWidget.h"
#include "viewSettingsWidget.h"
#include "../simulation/simulationEngine.h"
#include "../simulation/crystalPopulationRepository.h"

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

    GeneralSettingsWidget *mGeneralSettingsWidget;
    CrystalSettingsWidget *mCrystalSettingsWidget;
    ViewSettingsWidget *mViewSettingsWidget;
    QProgressBar *mProgressBar;
    RenderButton *mRenderButton;
    OpenGLWidget *mOpenGLWidget;

    std::shared_ptr<HaloSim::CrystalPopulationRepository> mCrystalRepository;
    std::shared_ptr<HaloSim::SimulationEngine> mEngine;
};
