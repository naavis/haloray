#pragma once
#include <QWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGroupBox>
#include "sliderSpinBox.h"
#include "../simulation/lightSource.h"

class GeneralSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    GeneralSettingsWidget(QWidget *parent = nullptr);
    void SetInitialValues(double sunDiameter,
                          double sunAltitude,
                          unsigned int raysPerFrame,
                          unsigned int maxNumFrames);

signals:
    void lightSourceChanged(HaloSim::LightSource light);
    void numRaysChanged(unsigned int rays);
    void maximumNumberOfIterationsChanged(unsigned int iterations);

public slots:
    void toggleMaxIterationsSpinBoxStatus();

private:
    HaloSim::LightSource stateToLightSource() const;
    void setupUi();

    SliderSpinBox *mSunAltitudeSlider;
    QDoubleSpinBox *mSunDiameterSpinBox;
    QSpinBox *mRaysPerFrameSpinBox;
    QSpinBox *mMaximumFramesSpinBox;
};
