#include "viewSettingsWidget.h"
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QDataWidgetMapper>
#include "simulationStateModel.h"
#include "sliderSpinBox.h"
#include "../simulation/camera.h"

namespace HaloRay
{

ViewSettingsWidget::ViewSettingsWidget(SimulationStateModel *viewModel, QWidget *parent)
    : QGroupBox("View settings", parent),
      m_viewModel(viewModel)
{
    setupUi();

    m_mapper = new QDataWidgetMapper(this);
    m_mapper->setModel(m_viewModel);
    m_mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    m_mapper->addMapping(m_cameraProjectionComboBox, SimulationStateModel::CameraProjection, "currentIndex");
    m_mapper->addMapping(m_fieldOfViewSlider, SimulationStateModel::CameraFov);
    m_mapper->addMapping(m_pitchSlider, SimulationStateModel::CameraPitch);
    m_mapper->addMapping(m_yawSlider, SimulationStateModel::CameraYaw);
    m_mapper->addMapping(m_hideSubHorizonCheckBox, SimulationStateModel::HideSubHorizon);
    m_mapper->toFirst();

    connect(m_cameraProjectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_fieldOfViewSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_pitchSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_yawSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_hideSubHorizonCheckBox, &QCheckBox::stateChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);

    /*
     * It is not possible to map multiple model columns to different properties
     * of the same widget with QDataWidgetMapper, so a second instance is needed
     * to update the maximum value of the FOV slider.
     */
    m_maximumFovMapper = new QDataWidgetMapper(this);
    m_maximumFovMapper->setModel(m_viewModel);
    m_maximumFovMapper->addMapping(m_fieldOfViewSlider, SimulationStateModel::CameraMaxFov, "maximum");
    m_maximumFovMapper->toFirst();

    connect(m_brightnessSlider, &SliderSpinBox::valueChanged, this, &ViewSettingsWidget::brightnessChanged);
    connect(m_lockToLightSource, &QCheckBox::stateChanged, this, &ViewSettingsWidget::lockToLightSource);
}

void ViewSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    m_cameraProjectionComboBox = new QComboBox();
    m_cameraProjectionComboBox->addItems({tr("Stereographic"),
                                         tr("Rectilinear"),
                                         tr("Equidistant"),
                                         tr("Equal area"),
                                         tr("Orthographic")});

    m_pitchSlider = SliderSpinBox::createAngleSlider(-90.0, 90.0);

    m_yawSlider = SliderSpinBox::createAngleSlider(-360.0, 360.0);
    m_yawSlider->setWrapping(true);

    m_fieldOfViewSlider = SliderSpinBox::createAngleSlider(10.0, 360.0);

    m_brightnessSlider = new SliderSpinBox();
    m_brightnessSlider->setMinimum(0.1);
    m_brightnessSlider->setMaximum(15.0);

    m_hideSubHorizonCheckBox = new QCheckBox();

    m_lockToLightSource = new QCheckBox();

    auto layout = new QFormLayout(this);
    layout->addRow(tr("Camera projection"), m_cameraProjectionComboBox);
    layout->addRow(tr("Field of view"), m_fieldOfViewSlider);
    layout->addRow(tr("Pitch"), m_pitchSlider);
    layout->addRow(tr("Yaw"), m_yawSlider);
    layout->addRow(tr("Brightness"), m_brightnessSlider);
    layout->addRow(tr("Hide sub-horizon"), m_hideSubHorizonCheckBox);
    layout->addRow(tr("Lock to light source"), m_lockToLightSource);
}

void ViewSettingsWidget::setBrightness(double brightness)
{
    m_brightnessSlider->setValue(brightness);
}

}
