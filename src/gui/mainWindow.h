#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include "openGLWidget.h"
#include "generalSettingsWidget.h"
#include "../simulation/simulationEngine.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

private:
    void setupUi();

    GeneralSettingsWidget *mGeneralSettingsWidget;
    QDoubleSpinBox *mCaRatioSpinBox;
    QDoubleSpinBox *mCaRatioStdSpinBox;
    QComboBox *mCameraProjectionComboBox;
    QPushButton *mRenderButton;
    OpenGLWidget *mOpenGLWidget;

    std::shared_ptr<HaloSim::SimulationEngine> mEngine;
};
