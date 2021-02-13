#pragma once
#include "collapsibleBox.h"
#include <memory>

class QToolButton;
class QComboBox;
class QLabel;
class QSpinBox;
class QDataWidgetMapper;
class QDoubleSpinBox;

namespace HaloRay
{

class AddCrystalPopulationButton;
class SliderSpinBox;
class CrystalModel;

class CrystalSettingsWidget : public CollapsibleBox
{
    Q_OBJECT
public:
    CrystalSettingsWidget(CrystalModel *model, QWidget *parent = nullptr);

private:
    void setupUi();
    void setupPopulationComboBoxConnections();
    SliderSpinBox *createAngleSlider(double min, double max);
    void setTiltVisibility(bool);
    void setRotationVisibility(bool);

    AddCrystalPopulationButton *m_addPopulationButton;
    QToolButton *m_removePopulationButton;

    SliderSpinBox *m_caRatioSlider;
    SliderSpinBox *m_caRatioStdSlider;

    QComboBox *m_tiltDistributionComboBox;
    SliderSpinBox *m_tiltAverageSlider;
    QLabel *m_tiltAverageLabel;
    SliderSpinBox *m_tiltStdSlider;
    QLabel *m_tiltStdLabel;

    QComboBox *m_rotationDistributionComboBox;
    SliderSpinBox *m_rotationAverageSlider;
    QLabel *m_rotationAverageLabel;
    SliderSpinBox *m_rotationStdSlider;
    QLabel *m_rotationStdLabel;

    QDoubleSpinBox *m_upperApexAngleSpinBox;
    SliderSpinBox *m_upperApexHeightAverageSlider;
    SliderSpinBox *m_upperApexHeightStdSlider;

    QDoubleSpinBox *m_lowerApexAngleSpinBox;
    SliderSpinBox *m_lowerApexHeightAverageSlider;
    SliderSpinBox *m_lowerApexHeightStdSlider;

    QSpinBox *m_weightSpinBox;

    CrystalModel *m_model;
    QDataWidgetMapper *m_mapper;
    QComboBox *m_populationComboBox;
};

}
