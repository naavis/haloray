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
    void setInitialValues(unsigned int raysPerFrame,
                          unsigned int maxNumFrames);


signals:
    void numRaysChanged(unsigned int rays);
    void maximumNumberOfIterationsChanged(unsigned int iterations);

public slots:
    void toggleMaxIterationsSpinBoxStatus();
    void setMaxRaysPerFrame(unsigned int maxRays);

private:
    LightSource stateToLightSource() const;
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
