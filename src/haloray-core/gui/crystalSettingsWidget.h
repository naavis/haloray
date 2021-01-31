#pragma once
#include <QGroupBox>
#include <memory>


class QToolButton;
class QComboBox;
class QLabel;
class QSpinBox;
class QDataWidgetMapper;

namespace HaloRay
{

class CrystalPopulationRepository;
class AddCrystalPopulationButton;
class SliderSpinBox;
class CrystalModel;

class CrystalSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    CrystalSettingsWidget(std::shared_ptr<CrystalPopulationRepository> crystalRepository, QWidget *parent = nullptr);

signals:
    void crystalChanged();

private:
    void setupUi();
    void addPopulationComboBoxItem();
    void fillPopulationComboBox();
    void updateRemovePopulationButtonState();
    SliderSpinBox *createAngleSlider(double min, double max);
    void setTiltVisibility(bool);
    void setRotationVisibility(bool);

    AddCrystalPopulationButton *m_AddPopulationButton;
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

    QSpinBox *m_weightSpinBox;

    CrystalModel *m_model;
    QDataWidgetMapper *m_mapper;
    QComboBox *m_populationComboBox;

    unsigned int m_nextPopulationId;
};

}
