#include "generalSettingsWidget.h"
#include <QFormLayout>
#include <QDoubleSpinBox>
#include "sliderSpinBox.h"
#include "../simulation/lightSource.h"

namespace HaloRay
{

GeneralSettingsWidget::GeneralSettingsWidget(SimulationStateModel *viewModel, QWidget *parent)
    : QGroupBox("General settings", parent),
      m_viewModel(viewModel)
{
    setupUi();

    m_mapper = new QDataWidgetMapper(this);
    m_mapper->setModel(m_viewModel);
    m_mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    m_mapper->addMapping(m_sunAltitudeSlider, SimulationStateModel::SunAltitude);
    m_mapper->addMapping(m_sunDiameterSpinBox, SimulationStateModel::SunDiameter);
    m_mapper->addMapping(m_multipleScatteringSlider, SimulationStateModel::MultipleScatteringProbability);
    m_mapper->toFirst();

    connect(m_sunAltitudeSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_sunDiameterSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_multipleScatteringSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);

    connect(m_raysPerFrameSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        emit numRaysChanged((unsigned int)value);
    });

    connect(m_maximumFramesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        emit maximumNumberOfIterationsChanged((unsigned int)value);
    });
}

void GeneralSettingsWidget::setInitialValues(unsigned int raysPerFrame,
                                             unsigned int maxNumFrames)
{
    m_raysPerFrameSpinBox->setValue(raysPerFrame);
    m_maximumFramesSpinBox->setValue(maxNumFrames);
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

    m_multipleScatteringSlider = new SliderSpinBox();
    m_multipleScatteringSlider->setMinimum(0.0);
    m_multipleScatteringSlider->setMaximum(1.0);

    auto layout = new QFormLayout(this);
    layout->addRow(tr("Sun altitude"), m_sunAltitudeSlider);
    layout->addRow(tr("Sun diameter"), m_sunDiameterSpinBox);
    layout->addRow(tr("Rays per frame"), m_raysPerFrameSpinBox);
    layout->addRow(tr("Maximum frames"), m_maximumFramesSpinBox);
    layout->addRow(tr("Double scattering"), m_multipleScatteringSlider);
}

LightSource GeneralSettingsWidget::stateToLightSource() const
{
    LightSource light;
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
