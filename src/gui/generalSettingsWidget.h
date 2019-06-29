#pragma once
#include <QWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "sliderSpinBox.h"
#include "../simulation/lightSource.h"

class GeneralSettingsWidget : public QWidget
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

private:
    HaloSim::LightSource stateToLightSource() const;

    SliderSpinBox *mSunAltitudeSlider;
    QDoubleSpinBox *mSunDiameterSpinBox;
    QSpinBox *mRaysPerFrameSpinBox;
    QSpinBox *mMaximumFramesSpinBox;
};
