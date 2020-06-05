#pragma once
#include <QGroupBox>
#include "../simulation/camera.h"

class QComboBox;
class QCheckBox;
class SliderSpinBox;

class ViewSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    ViewSettingsWidget(QWidget *parent = nullptr);

    void setCamera(HaloSim::Camera camera);
    void setFieldOfView(double fov);
    void setCameraOrientation(double pitch, double yaw);
    void setBrightness(double brightness);

signals:
    void cameraChanged(HaloSim::Camera camera);
    void brightnessChanged(double brightness);
    void lockToLightSource(bool locked);

private:
    void setupUi();
    HaloSim::Camera stateToCamera() const;
    SliderSpinBox *mFieldOfViewSlider;
    SliderSpinBox *mPitchSlider;
    SliderSpinBox *mYawSlider;
    QComboBox *mCameraProjectionComboBox;
    QCheckBox *mHideSubHorizonCheckBox;
    SliderSpinBox *mBrightnessSlider;
    QCheckBox *mLockToLightSource;
};
