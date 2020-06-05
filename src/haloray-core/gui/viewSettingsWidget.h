#pragma once
#include <QGroupBox>
#include "../simulation/camera.h"

class QComboBox;
class QCheckBox;

namespace HaloRay
{

class SliderSpinBox;

class ViewSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    ViewSettingsWidget(QWidget *parent = nullptr);

    void setCamera(HaloRay::Camera camera);
    void setFieldOfView(double fov);
    void setCameraOrientation(double pitch, double yaw);
    void setBrightness(double brightness);

signals:
    void cameraChanged(HaloRay::Camera camera);
    void brightnessChanged(double brightness);
    void lockToLightSource(bool locked);

private:
    void setupUi();
    HaloRay::Camera stateToCamera() const;
    SliderSpinBox *mFieldOfViewSlider;
    SliderSpinBox *mPitchSlider;
    SliderSpinBox *mYawSlider;
    QComboBox *mCameraProjectionComboBox;
    QCheckBox *mHideSubHorizonCheckBox;
    SliderSpinBox *mBrightnessSlider;
    QCheckBox *mLockToLightSource;
};

}
