#pragma once
#include <QGroupBox>
#include <QDataWidgetMapper>
#include "../simulation/lightSource.h"
#include "simulationStateModel.h"

class QDoubleSpinBox;
class QSpinBox;

namespace HaloRay
{

class SliderSpinBox;

class GeneralSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    GeneralSettingsWidget(SimulationStateModel *viewModel, QWidget *parent = nullptr);

public slots:
    void toggleComputeShaderParametersEnabled();

private:
    void setupUi();

    SliderSpinBox *m_sunAltitudeSlider;
    QDoubleSpinBox *m_sunDiameterSpinBox;
    QSpinBox *m_raysPerFrameSpinBox;
    QSpinBox *m_maximumFramesSpinBox;
    SliderSpinBox *m_multipleScatteringSlider;

    QDataWidgetMapper *m_mapper;
    SimulationStateModel *m_viewModel;
};

}
