#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include "openGLWidget.h"
#include "generalSettingsWidget.h"
#include "crystalSettingsWidget.h"
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
    QComboBox *mCameraProjectionComboBox;
    QPushButton *mRenderButton;
    OpenGLWidget *mOpenGLWidget;

    std::shared_ptr<HaloSim::SimulationEngine> mEngine;
};
