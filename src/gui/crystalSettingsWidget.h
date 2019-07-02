#pragma once
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QDataWidgetMapper>
#include "sliderSpinBox.h"
#include "crystalModel.h"
#include "../simulation/crystalPopulation.h"

class CrystalSettingsWidget : public QGroupBox
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
    QLabel *mTiltAverageLabel;
    SliderSpinBox *mTiltStdSlider;
    QLabel *mTiltStdLabel;

    QComboBox *mRotationDistributionComboBox;
    SliderSpinBox *mRotationAverageSlider;
    QLabel *mRotationAverageLabel;
    SliderSpinBox *mRotationStdSlider;
    QLabel *mRotationStdLabel;

    CrystalModel *mModel;
    QDataWidgetMapper *mMapper;
};
