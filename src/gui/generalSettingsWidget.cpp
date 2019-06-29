#include "generalSettingsWidget.h"
#include <QLabel>
#include <QFormLayout>

GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent) : QWidget(parent)
{
    mSunAltitudeSlider = new SliderSpinBox(this);
    mSunAltitudeSlider->setSuffix("­°");
    mSunAltitudeSlider->setMinimum(-90.0);
    mSunAltitudeSlider->setMaximum(90.0);
    mSunAltitudeSlider->setValue(0.0);

    mSunDiameterSpinBox = new QDoubleSpinBox(this);
    mSunDiameterSpinBox->setSuffix("­°");
    mSunDiameterSpinBox->setSingleStep(0.1);
    mSunDiameterSpinBox->setMinimum(0.01);
    mSunDiameterSpinBox->setMaximum(30.0);
    mSunDiameterSpinBox->setValue(0.5);

    mRaysPerFrameSpinBox = new QSpinBox(this);
    mRaysPerFrameSpinBox->setSingleStep(1000);
    mRaysPerFrameSpinBox->setMinimum(1);
    mRaysPerFrameSpinBox->setMaximum(2000000000);
    mRaysPerFrameSpinBox->setValue(500000);

    mMaximumFramesSpinBox = new QSpinBox(this);
    mRaysPerFrameSpinBox->setSingleStep(60);
    mMaximumFramesSpinBox->setMinimum(1);
    mMaximumFramesSpinBox->setMaximum(1000000000);
    mMaximumFramesSpinBox->setValue(100000000);

    connect(mSunAltitudeSlider, &SliderSpinBox::valueChanged, [=](double value) {
        lightSourceChanged(stateToLightSource());
    });

    connect(mSunDiameterSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
        lightSourceChanged(stateToLightSource());
    });

    connect(mRaysPerFrameSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
        numRaysChanged((unsigned int)value);
    });

    auto layout = new QFormLayout(this);
    layout->addRow("Sun altitude", mSunAltitudeSlider);
    layout->addRow("Sun diameter", mSunDiameterSpinBox);
    layout->addRow("Rays per frame", mRaysPerFrameSpinBox);
    layout->addRow("Maximum frames", mMaximumFramesSpinBox);
    this->setLayout(layout);
}

void GeneralSettingsWidget::SetInitialValues(double sunDiameter,
                                             double sunAltitude,
                                             unsigned int raysPerFrame,
                                             unsigned int maxNumFrames)
{
    qDebug("Setting initial values for general settings widget");
    qDebug("Setting sun diameter");
    mSunDiameterSpinBox->setValue(sunDiameter);
    qDebug("Setting sun altitude");
    mSunAltitudeSlider->setValue(sunAltitude);
    qDebug("Settings rays per frame");
    mRaysPerFrameSpinBox->setValue(raysPerFrame);
    qDebug("Setting max number of frames");
    mMaximumFramesSpinBox->setValue(maxNumFrames);
}

HaloSim::LightSource GeneralSettingsWidget::stateToLightSource() const
{
    HaloSim::LightSource light;
    light.altitude = mSunAltitudeSlider->value();
    light.diameter = mSunDiameterSpinBox->value();
    return light;
}
