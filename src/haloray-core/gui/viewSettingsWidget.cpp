#include "viewSettingsWidget.h"
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include "sliderSpinBox.h"
#include "../simulation/camera.h"

namespace HaloRay
{

ViewSettingsWidget::ViewSettingsWidget(QWidget *parent)
    : QGroupBox("View settings", parent)
{
    setupUi();

    auto cameraChangeHandler = [this]() {
        auto camera = stateToCamera();
        m_fieldOfViewSlider->setMaximum(camera.getMaximumFov());
        camera.fov = m_fieldOfViewSlider->value();
        cameraChanged(camera);
    };

    connect(m_cameraProjectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), cameraChangeHandler);
    connect(m_fieldOfViewSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(m_pitchSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(m_yawSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(m_hideSubHorizonCheckBox, &QCheckBox::stateChanged, cameraChangeHandler);
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

HaloRay::Camera ViewSettingsWidget::stateToCamera() const
{
    HaloRay::Camera camera;
    camera.projection = (HaloRay::Projection)m_cameraProjectionComboBox->currentIndex();
    camera.fov = (float)m_fieldOfViewSlider->value();
    camera.hideSubHorizon = m_hideSubHorizonCheckBox->isChecked();
    camera.yaw = (float)m_yawSlider->value();
    camera.pitch = (float)m_pitchSlider->value();
    return camera;
}

void ViewSettingsWidget::setCamera(HaloRay::Camera camera)
{
    m_cameraProjectionComboBox->setCurrentIndex((int)camera.projection);
    m_fieldOfViewSlider->setValue(camera.fov);
    m_yawSlider->setValue(camera.yaw);
    m_pitchSlider->setValue(camera.pitch);
    m_hideSubHorizonCheckBox->setChecked(camera.hideSubHorizon);
}

void ViewSettingsWidget::setFieldOfView(double fov)
{
    m_fieldOfViewSlider->setValue(fov);
}

void ViewSettingsWidget::setCameraOrientation(double pitch, double yaw)
{
    m_pitchSlider->setValue(pitch);
    m_yawSlider->setValue(yaw);
}

void ViewSettingsWidget::setBrightness(double brightness)
{
    m_brightnessSlider->setValue(brightness);
}

}
