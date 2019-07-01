#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include "renderButton.h"
#include "openGLWidget.h"
#include "generalSettingsWidget.h"
#include "crystalSettingsWidget.h"
#include "viewSettingsWidget.h"
#include "../simulation/simulationEngine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

private:
    void setupUi();

    GeneralSettingsWidget *mGeneralSettingsWidget;
    CrystalSettingsWidget *mCrystalSettingsWidget;
    ViewSettingsWidget *mViewSettingsWidget;
    RenderButton *mRenderButton;
    OpenGLWidget *mOpenGLWidget;

    std::shared_ptr<HaloSim::SimulationEngine> mEngine;
};
