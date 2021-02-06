#pragma once

#include <QGroupBox>
#include <QObject>

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
    SliderSpinBox *m_turbiditySlider;
    SliderSpinBox *m_groundAlbedoSlider;
    QDataWidgetMapper *m_mapper;
};

}
