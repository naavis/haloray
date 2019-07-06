#include "viewSettingsWidget.h"
#include <QFormLayout>

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
    mCameraProjectionComboBox->addItems({"Stereographic",
                                         "Rectilinear",
                                         "Equidistant",
                                         "Equal area",
                                         "Orthographic"});

    mPitchSlider = new SliderSpinBox();
    mPitchSlider->setMinimum(-90.0);
    mPitchSlider->setMaximum(90.0);
    mPitchSlider->setSuffix("°");

    mYawSlider = new SliderSpinBox();
    mYawSlider->setMinimum(-360.0);
    mYawSlider->setMaximum(360.0);
    mYawSlider->setWrapping(true);
    mYawSlider->setSuffix("°");

    mFieldOfViewSlider = new SliderSpinBox();
    mFieldOfViewSlider->setMinimum(10.0);
    mFieldOfViewSlider->setMaximum(360.0);
    mFieldOfViewSlider->setSuffix("°");

    mBrightnessSlider = new SliderSpinBox();
    mBrightnessSlider->setMinimum(0.1);
    mBrightnessSlider->setMaximum(15.0);

    mHideSubHorizonCheckBox = new QCheckBox();

    mLockToLightSource = new QCheckBox();

    auto layout = new QFormLayout(this);
    layout->addRow("Camera projection", mCameraProjectionComboBox);
    layout->addRow("Field of view", mFieldOfViewSlider);
    layout->addRow("Pitch", mPitchSlider);
    layout->addRow("Yaw", mYawSlider);
    layout->addRow("Brightness", mBrightnessSlider);
    layout->addRow("Hide sub-horizon", mHideSubHorizonCheckBox);
    layout->addRow("Lock to light source", mLockToLightSource);
}

HaloSim::Camera ViewSettingsWidget::stateToCamera() const
{
    HaloSim::Camera camera;
    camera.projection = (HaloSim::Projection)mCameraProjectionComboBox->currentIndex();
    camera.fov = (float)mFieldOfViewSlider->value();
    camera.hideSubHorizon = mHideSubHorizonCheckBox->isChecked();
    camera.yaw = (float)mYawSlider->value();
    camera.pitch = (float)mPitchSlider->value();
    return camera;
}

void ViewSettingsWidget::setCamera(HaloSim::Camera camera)
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
