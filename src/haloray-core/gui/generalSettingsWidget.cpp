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

    connect(m_sunAltitudeSlider, &SliderSpinBox::valueChanged, [this]() {
        emit lightSourceChanged(stateToLightSource());
    });

    connect(m_sunDiameterSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this]() {
        emit lightSourceChanged(stateToLightSource());
    });

    connect(m_raysPerFrameSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        emit numRaysChanged((unsigned int)value);
    });

    connect(m_maximumFramesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        emit maximumNumberOfIterationsChanged((unsigned int)value);
    });

    connect(m_multipleScattering, &SliderSpinBox::valueChanged, this, &GeneralSettingsWidget::multipleScatteringProbabilityChanged);
}

void GeneralSettingsWidget::setInitialValues(double sunDiameter,
                                             double sunAltitude,
                                             unsigned int raysPerFrame,
                                             unsigned int maxNumFrames,
                                             double multipleScatteringProbability)
{
    m_sunDiameterSpinBox->setValue(sunDiameter);
    m_sunAltitudeSlider->setValue(sunAltitude);
    m_raysPerFrameSpinBox->setValue(raysPerFrame);
    m_maximumFramesSpinBox->setValue(maxNumFrames);
    m_multipleScattering->setValue(multipleScatteringProbability);
}

void GeneralSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    m_sunAltitudeSlider = new SliderSpinBox();
    m_sunAltitudeSlider = SliderSpinBox::createAngleSlider(-90.0, 90.0);

    m_sunDiameterSpinBox = new QDoubleSpinBox();
    m_sunDiameterSpinBox->setSuffix("­°");
    m_sunDiameterSpinBox->setSingleStep(0.1);
    m_sunDiameterSpinBox->setMinimum(0.01);
    m_sunDiameterSpinBox->setMaximum(30.0);
    m_sunDiameterSpinBox->setValue(0.5);

    m_raysPerFrameSpinBox = new QSpinBox();
    m_raysPerFrameSpinBox->setSingleStep(1000);
    m_raysPerFrameSpinBox->setMinimum(1);
    m_raysPerFrameSpinBox->setMaximum(2000000000);
    m_raysPerFrameSpinBox->setValue(500000);
    m_raysPerFrameSpinBox->setGroupSeparatorShown(true);

    m_maximumFramesSpinBox = new QSpinBox();
    m_raysPerFrameSpinBox->setSingleStep(60);
    m_maximumFramesSpinBox->setMinimum(100);
    m_maximumFramesSpinBox->setMaximum(1000000000);
    m_maximumFramesSpinBox->setValue(100000000);
    m_maximumFramesSpinBox->setGroupSeparatorShown(true);

    m_multipleScattering = new SliderSpinBox();
    m_multipleScattering->setMinimum(0.0);
    m_multipleScattering->setMaximum(1.0);

    auto layout = new QFormLayout(this);
    layout->addRow(tr("Sun altitude"), m_sunAltitudeSlider);
    layout->addRow(tr("Sun diameter"), m_sunDiameterSpinBox);
    layout->addRow(tr("Rays per frame"), m_raysPerFrameSpinBox);
    layout->addRow(tr("Maximum frames"), m_maximumFramesSpinBox);
    layout->addRow(tr("Double scattering"), m_multipleScattering);
}

HaloRay::LightSource GeneralSettingsWidget::stateToLightSource() const
{
    HaloRay::LightSource light;
    light.altitude = m_sunAltitudeSlider->value();
    light.diameter = m_sunDiameterSpinBox->value();
    return light;
}

void GeneralSettingsWidget::toggleMaxIterationsSpinBoxStatus()
{
    m_maximumFramesSpinBox->setEnabled(!m_maximumFramesSpinBox->isEnabled());
}

void GeneralSettingsWidget::setMaxRaysPerFrame(unsigned int maxRays)
{
    m_raysPerFrameSpinBox->setMaximum((int)maxRays);
}

}
