#pragma once
#include <QGroupBox>
#include "../simulation/camera.h"

class QComboBox;
class QCheckBox;
class QDataWidgetMapper;

namespace HaloRay
{

class SliderSpinBox;
class SimulationStateViewModel;

class ViewSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    ViewSettingsWidget(SimulationStateViewModel *viewModel, QWidget *parent = nullptr);
    void setBrightness(double brightness);

signals:
    void brightnessChanged(double brightness);
    void lockToLightSource(bool locked);

private:
    void setupUi();

    SliderSpinBox *m_fieldOfViewSlider;
    SliderSpinBox *m_pitchSlider;
    SliderSpinBox *m_yawSlider;
    QComboBox *m_cameraProjectionComboBox;
    QCheckBox *m_hideSubHorizonCheckBox;
    SliderSpinBox *m_brightnessSlider;
    QCheckBox *m_lockToLightSource;
    SimulationStateViewModel *m_viewModel;
    QDataWidgetMapper *m_mapper;
    QDataWidgetMapper *m_maximumFovMapper;
};

}
