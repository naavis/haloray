#include "atmosphereSettingsWidget.h"
#include <QFormLayout>
#include <QDataWidgetMapper>
#include <QCheckBox>
#include "simulationStateModel.h"
#include "sliderSpinBox.h"

namespace HaloRay
{

AtmosphereSettingsWidget::AtmosphereSettingsWidget(SimulationStateModel *viewModel, QWidget *parent)
    : CollapsibleBox("Atmosphere settings", false, parent),
      m_viewModel(viewModel)
{
    setupUi();

    m_mapper = new QDataWidgetMapper(this);
    m_mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    m_mapper->setModel(m_viewModel);
    m_mapper->addMapping(m_atmosphereEnabledCheckBox, SimulationStateModel::AtmosphereEnabled, "checked");
    m_mapper->addMapping(m_turbiditySlider, SimulationStateModel::Turbidity);
    m_mapper->addMapping(m_groundAlbedoSlider, SimulationStateModel::GroundAlbedo);
    m_mapper->toFirst();

    connect(m_atmosphereEnabledCheckBox, &QCheckBox::toggled, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_turbiditySlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_groundAlbedoSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
}

void AtmosphereSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    m_atmosphereEnabledCheckBox = new QCheckBox();

    m_turbiditySlider = new SliderSpinBox(1.0, 9.0);

    m_groundAlbedoSlider = new SliderSpinBox(0.0, 1.0);

    auto layout = new QFormLayout(this->contentWidget());
    layout->addRow(tr("Atmosphere enabled"), m_atmosphereEnabledCheckBox);
    layout->addRow(tr("Turbidity"), m_turbiditySlider);
    layout->addRow(tr("Ground albedo"), m_groundAlbedoSlider);
}

}
