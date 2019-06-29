#pragma once
#include <QWidget>
#include <QComboBox>
#include "sliderSpinBox.h"
#include "../simulation/crystalPopulation.h"

class CrystalSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    CrystalSettingsWidget(QWidget *parent = nullptr);

    void SetInitialValues(HaloSim::CrystalPopulation crystal);

signals:
    void crystalChanged(HaloSim::CrystalPopulation crystal);

private:
    void setupUi();
    HaloSim::CrystalPopulation stateToCrystalPopulation() const;

    SliderSpinBox *mCaRatioSlider;
    SliderSpinBox *mCaRatioStdSlider;

    QComboBox *mTiltDistributionComboBox;
    SliderSpinBox *mTiltAverageSlider;
    SliderSpinBox *mTiltStdSlider;

    QComboBox *mRotationDistributionComboBox;
    SliderSpinBox *mRotationAverageSlider;
    SliderSpinBox *mRotationStdSlider;
};
