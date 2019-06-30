#pragma once
#include <QGroupBox>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include "sliderSpinBox.h"
#include "../simulation/camera.h"

class ViewSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    ViewSettingsWidget(QWidget *parent = nullptr);

    void setCamera(HaloSim::Camera camera);
    void setFieldOfView(double fov);
    void setCameraOrientation(double pitch, double yaw);

signals:
    void cameraChanged(HaloSim::Camera camera);

private:
    void setupUi();
    HaloSim::Camera stateToCamera() const;
    SliderSpinBox *mFieldOfViewSlider;
    SliderSpinBox *mPitchSlider;
    SliderSpinBox *mYawSlider;
    QComboBox *mCameraProjectionComboBox;
    QCheckBox *mHideSubHorizonCheckBox;
};
