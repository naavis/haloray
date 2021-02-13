#pragma once

#include <QObject>
#include "collapsibleBox.h"

class QDataWidgetMapper;
class QCheckBox;

namespace HaloRay
{

class SimulationStateModel;
class SliderSpinBox;

class AtmosphereSettingsWidget : public CollapsibleBox
{
    Q_OBJECT
public:
    AtmosphereSettingsWidget(SimulationStateModel *viewModel, QWidget *parent = nullptr);

private:
    void setupUi();

    SimulationStateModel *m_viewModel;
    SliderSpinBox *m_turbiditySlider;
    SliderSpinBox *m_groundAlbedoSlider;
    QCheckBox *m_atmosphereEnabledCheckBox;

    QDataWidgetMapper *m_mapper;
};

}
