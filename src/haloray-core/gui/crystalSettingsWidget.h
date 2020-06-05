#pragma once
#include <QGroupBox>
#include <memory>

namespace HaloSim
{
class CrystalPopulationRepository;
}

class QToolButton;
class AddCrystalPopulationButton;
class SliderSpinBox;
class QComboBox;
class QLabel;
class QSpinBox;
class CrystalModel;
class QDataWidgetMapper;

class CrystalSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    CrystalSettingsWidget(std::shared_ptr<HaloSim::CrystalPopulationRepository> crystalRepository, QWidget *parent = nullptr);

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

    AddCrystalPopulationButton *mAddPopulationButton;
    QToolButton *mRemovePopulationButton;

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

    QSpinBox *mWeightSpinBox;

    CrystalModel *mModel;
    QDataWidgetMapper *mMapper;
    QComboBox *mPopulationComboBox;

    unsigned int mNextPopulationId;
};
