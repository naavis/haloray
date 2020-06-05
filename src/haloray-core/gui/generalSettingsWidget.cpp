#include "generalSettingsWidget.h"
#include <QFormLayout>
#include <QDoubleSpinBox>
#include "sliderSpinBox.h"
#include "../simulation/lightSource.h"

namespace HaloRay
{

GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent)
    : QGroupBox("General settings", parent)
{
    setupUi();

    connect(mSunAltitudeSlider, &SliderSpinBox::valueChanged, [this]() {
        emit lightSourceChanged(stateToLightSource());
    });

    connect(mSunDiameterSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this]() {
        emit lightSourceChanged(stateToLightSource());
    });

    connect(mRaysPerFrameSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        emit numRaysChanged((unsigned int)value);
    });

    connect(mMaximumFramesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        emit maximumNumberOfIterationsChanged((unsigned int)value);
    });

    connect(mMultipleScattering, &SliderSpinBox::valueChanged, this, &GeneralSettingsWidget::multipleScatteringProbabilityChanged);
}

void GeneralSettingsWidget::setInitialValues(double sunDiameter,
                                             double sunAltitude,
                                             unsigned int raysPerFrame,
                                             unsigned int maxNumFrames,
                                             double multipleScatteringProbability)
{
    mSunDiameterSpinBox->setValue(sunDiameter);
    mSunAltitudeSlider->setValue(sunAltitude);
    mRaysPerFrameSpinBox->setValue(raysPerFrame);
    mMaximumFramesSpinBox->setValue(maxNumFrames);
    mMultipleScattering->setValue(multipleScatteringProbability);
}

void GeneralSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    mSunAltitudeSlider = new SliderSpinBox();
    mSunAltitudeSlider = SliderSpinBox::createAngleSlider(-90.0, 90.0);

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

    mMultipleScattering = new SliderSpinBox();
    mMultipleScattering->setMinimum(0.0);
    mMultipleScattering->setMaximum(1.0);

    auto layout = new QFormLayout(this);
    layout->addRow(tr("Sun altitude"), mSunAltitudeSlider);
    layout->addRow(tr("Sun diameter"), mSunDiameterSpinBox);
    layout->addRow(tr("Rays per frame"), mRaysPerFrameSpinBox);
    layout->addRow(tr("Maximum frames"), mMaximumFramesSpinBox);
    layout->addRow(tr("Double scattering"), mMultipleScattering);
}

HaloRay::LightSource GeneralSettingsWidget::stateToLightSource() const
{
    HaloRay::LightSource light;
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

}
