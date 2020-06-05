#pragma once
#include <QGroupBox>
#include "../simulation/lightSource.h"

class QDoubleSpinBox;
class QSpinBox;
class SliderSpinBox;

class GeneralSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    GeneralSettingsWidget(QWidget *parent = nullptr);
    void setInitialValues(double sunDiameter,
                          double sunAltitude,
                          unsigned int raysPerFrame,
                          unsigned int maxNumFrames,
                          double multipleScatteringProbability);

signals:
    void lightSourceChanged(HaloSim::LightSource light);
    void numRaysChanged(unsigned int rays);
    void maximumNumberOfIterationsChanged(unsigned int iterations);
    void multipleScatteringProbabilityChanged(double probability);

public slots:
    void toggleMaxIterationsSpinBoxStatus();
    void setMaxRaysPerFrame(unsigned int maxRays);

private:
    HaloSim::LightSource stateToLightSource() const;
    void setupUi();

    SliderSpinBox *mSunAltitudeSlider;
    QDoubleSpinBox *mSunDiameterSpinBox;
    QSpinBox *mRaysPerFrameSpinBox;
    QSpinBox *mMaximumFramesSpinBox;
    SliderSpinBox *mMultipleScattering;
};
