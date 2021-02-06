#pragma once

#include <QGroupBox>
#include <QObject>

class QSpinBox;
class QDataWidgetMapper;

namespace HaloRay
{

class SimulationStateModel;
class SliderSpinBox;

class AtmosphereSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    AtmosphereSettingsWidget(SimulationStateModel *viewModel, QWidget *parent = nullptr);

private:
    void setupUi();

    SimulationStateModel *m_viewModel;
    QSpinBox *m_turbiditySpinBox;
    SliderSpinBox *m_groundAlbedoSlider;
    QDataWidgetMapper *m_mapper;
};

}
