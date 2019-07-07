#pragma once
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDataWidgetMapper>
#include "sliderSpinBox.h"
#include "crystalModel.h"
#include "../simulation/crystalPopulation.h"
#include "../simulation/crystalPopulationRepository.h"

class CrystalSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    CrystalSettingsWidget(std::shared_ptr<HaloSim::CrystalPopulationRepository> crystalRepository, QWidget *parent = nullptr);

signals:
    void crystalChanged();

private:
    void setupUi();
    SliderSpinBox *createAngleSlider(double min, double max);
    void setTiltVisibility(bool);
    void setRotationVisibility(bool);

    QPushButton *mAddPopulationButton;
    QPushButton *mRemovePopulationButton;

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
    QComboBox *mPopulationComboBox;

    unsigned int mNextPopulationId;
};
