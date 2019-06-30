#include "viewSettingsWidget.h"
#include <QFormLayout>

ViewSettingsWidget::ViewSettingsWidget(QWidget *parent)
    : QGroupBox("View settings", parent)
{
    setupUi();

    auto cameraChangeHandler = [this]() { cameraChanged(stateToCamera()); };

    connect(mCameraProjectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), cameraChangeHandler);
    connect(mFieldOfViewSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(mPitchSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(mYawSlider, &SliderSpinBox::valueChanged, cameraChangeHandler);
    connect(mHideSubHorizonCheckBox, &QCheckBox::stateChanged, cameraChangeHandler);
}

void ViewSettingsWidget::setupUi()
{
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
    mFieldOfViewSlider->setMinimum(0.01);
    mFieldOfViewSlider->setMaximum(2.0);

    mHideSubHorizonCheckBox = new QCheckBox();

    auto layout = new QFormLayout(this);
    layout->addRow("Camera projection", mCameraProjectionComboBox);
    layout->addRow("Field of view", mFieldOfViewSlider);
    layout->addRow("Pitch", mPitchSlider);
    layout->addRow("Yaw", mYawSlider);
    layout->addRow("Hide sub-horizon", mHideSubHorizonCheckBox);
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
