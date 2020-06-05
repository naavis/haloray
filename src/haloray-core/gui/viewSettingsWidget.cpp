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
        mFieldOfViewSlider->setMaximum(camera.getMaximumFov());
        camera.fov = mFieldOfViewSlider->value();
        cameraChanged(camera);
    };

    connect(mCameraProjectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), cameraChangeHandler);
    connect(mFieldOfViewSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(mPitchSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(mYawSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(mHideSubHorizonCheckBox, &QCheckBox::stateChanged, cameraChangeHandler);
    connect(mBrightnessSlider, &SliderSpinBox::valueChanged, this, &ViewSettingsWidget::brightnessChanged);
    connect(mLockToLightSource, &QCheckBox::stateChanged, this, &ViewSettingsWidget::lockToLightSource);
}

void ViewSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    mCameraProjectionComboBox = new QComboBox();
    mCameraProjectionComboBox->addItems({tr("Stereographic"),
                                         tr("Rectilinear"),
                                         tr("Equidistant"),
                                         tr("Equal area"),
                                         tr("Orthographic")});

    mPitchSlider = SliderSpinBox::createAngleSlider(-90.0, 90.0);

    mYawSlider = SliderSpinBox::createAngleSlider(-360.0, 360.0);
    mYawSlider->setWrapping(true);

    mFieldOfViewSlider = SliderSpinBox::createAngleSlider(10.0, 360.0);

    mBrightnessSlider = new SliderSpinBox();
    mBrightnessSlider->setMinimum(0.1);
    mBrightnessSlider->setMaximum(15.0);

    mHideSubHorizonCheckBox = new QCheckBox();

    mLockToLightSource = new QCheckBox();

    auto layout = new QFormLayout(this);
    layout->addRow(tr("Camera projection"), mCameraProjectionComboBox);
    layout->addRow(tr("Field of view"), mFieldOfViewSlider);
    layout->addRow(tr("Pitch"), mPitchSlider);
    layout->addRow(tr("Yaw"), mYawSlider);
    layout->addRow(tr("Brightness"), mBrightnessSlider);
    layout->addRow(tr("Hide sub-horizon"), mHideSubHorizonCheckBox);
    layout->addRow(tr("Lock to light source"), mLockToLightSource);
}

HaloRay::Camera ViewSettingsWidget::stateToCamera() const
{
    HaloRay::Camera camera;
    camera.projection = (HaloRay::Projection)mCameraProjectionComboBox->currentIndex();
    camera.fov = (float)mFieldOfViewSlider->value();
    camera.hideSubHorizon = mHideSubHorizonCheckBox->isChecked();
    camera.yaw = (float)mYawSlider->value();
    camera.pitch = (float)mPitchSlider->value();
    return camera;
}

void ViewSettingsWidget::setCamera(HaloRay::Camera camera)
{
    mCameraProjectionComboBox->setCurrentIndex((int)camera.projection);
    mFieldOfViewSlider->setValue(camera.fov);
    mYawSlider->setValue(camera.yaw);
    mPitchSlider->setValue(camera.pitch);
    mHideSubHorizonCheckBox->setChecked(camera.hideSubHorizon);
}

void ViewSettingsWidget::setFieldOfView(double fov)
{
    mFieldOfViewSlider->setValue(fov);
}

void ViewSettingsWidget::setCameraOrientation(double pitch, double yaw)
{
    mPitchSlider->setValue(pitch);
    mYawSlider->setValue(yaw);
}

void ViewSettingsWidget::setBrightness(double brightness)
{
    mBrightnessSlider->setValue(brightness);
}

}
