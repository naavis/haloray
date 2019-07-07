#include "generalSettingsWidget.h"
#include <QFormLayout>

GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent)
    : QGroupBox("General settings", parent)
{
    setupUi();

    connect(mSunAltitudeSlider, &SliderSpinBox::valueChanged, [this]() {
        lightSourceChanged(stateToLightSource());
    });

    connect(mSunDiameterSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this]() {
        lightSourceChanged(stateToLightSource());
    });

    connect(mRaysPerFrameSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        numRaysChanged((unsigned int)value);
    });

    connect(mMaximumFramesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        maximumNumberOfIterationsChanged((unsigned int)value);
    });
}

void GeneralSettingsWidget::SetInitialValues(double sunDiameter,
                                             double sunAltitude,
                                             unsigned int raysPerFrame,
                                             unsigned int maxNumFrames)
{
    mSunDiameterSpinBox->setValue(sunDiameter);
    mSunAltitudeSlider->setValue(sunAltitude);
    mRaysPerFrameSpinBox->setValue(raysPerFrame);
    mMaximumFramesSpinBox->setValue(maxNumFrames);
}

void GeneralSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    mSunAltitudeSlider = new SliderSpinBox();
    mSunAltitudeSlider->setSuffix("­°");
    mSunAltitudeSlider->setMinimum(-90.0);
    mSunAltitudeSlider->setMaximum(90.0);
    mSunAltitudeSlider->setValue(0.0);

    mSunDiameterSpinBox = new QDoubleSpinBox();
    mSunDiameterSpinBox->setSuffix("­°");
    mSunDiameterSpinBox->setSingleStep(0.1);
    mSunDiameterSpinBox->setMinimum(0.01);
    mSunDiameterSpinBox->setMaximum(30.0);
    mSunDiameterSpinBox->setValue(0.5);

    mRaysPerFrameSpinBox = new QSpinBox();
    mRaysPerFrameSpinBox->setSingleStep(1000);
    mRaysPerFrameSpinBox->setMinimum(1);
    mRaysPerFrameSpinBox->setMaximum(2000000000);
    mRaysPerFrameSpinBox->setValue(500000);
    mRaysPerFrameSpinBox->setGroupSeparatorShown(true);

    mMaximumFramesSpinBox = new QSpinBox();
    mRaysPerFrameSpinBox->setSingleStep(60);
    mMaximumFramesSpinBox->setMinimum(100);
    mMaximumFramesSpinBox->setMaximum(1000000000);
    mMaximumFramesSpinBox->setValue(100000000);
    mMaximumFramesSpinBox->setGroupSeparatorShown(true);

    auto layout = new QFormLayout(this);
    layout->addRow("Sun altitude", mSunAltitudeSlider);
    layout->addRow("Sun diameter", mSunDiameterSpinBox);
    layout->addRow("Rays per frame", mRaysPerFrameSpinBox);
    layout->addRow("Maximum frames", mMaximumFramesSpinBox);
}

HaloSim::LightSource GeneralSettingsWidget::stateToLightSource() const
{
    HaloSim::LightSource light;
    light.altitude = mSunAltitudeSlider->value();
    light.diameter = mSunDiameterSpinBox->value();
    return light;
}

void GeneralSettingsWidget::toggleMaxIterationsSpinBoxStatus()
{
    mMaximumFramesSpinBox->setEnabled(!mMaximumFramesSpinBox->isEnabled());
}

void GeneralSettingsWidget::setMaxRaysPerFrame(unsigned int maxRays)
{
    mRaysPerFrameSpinBox->setMaximum((int)maxRays);
}
