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

signals:
    void lightSourceChanged(HaloSim::LightSource light);

private:
    HaloSim::LightSource stateToLightSource() const;

    SliderSpinBox *mSunAltitudeSlider;
    QDoubleSpinBox *mSunDiameterSpinBox;
    QSpinBox *mRaysPerFrameSpinBox;
    QSpinBox *mMaximumFramesSpinBox;
};
