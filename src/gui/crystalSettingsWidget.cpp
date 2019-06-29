#include "crystalSettingsWidget.h"
#include <QFormLayout>
#include "../simulation/crystalPopulation.h"

CrystalSettingsWidget::CrystalSettingsWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();

    connect(mCaRatioSlider, &SliderSpinBox::valueChanged, [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });
    connect(mCaRatioStdSlider, &SliderSpinBox::valueChanged, [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });

    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });
    connect(mTiltAverageSlider, &SliderSpinBox::valueChanged, [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });
    connect(mTiltStdSlider, &SliderSpinBox::valueChanged, [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });

    connect(mRotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });
    connect(mRotationAverageSlider, &SliderSpinBox::valueChanged, [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });
    connect(mRotationStdSlider, &SliderSpinBox::valueChanged, [this]() {
        crystalChanged(this->stateToCrystalPopulation());
    });

    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if (index == 0)
        {
            this->mTiltAverageSlider->setVisible(false);
            this->mTiltStdSlider->setVisible(false);
        }
        else
        {
            this->mTiltAverageSlider->setVisible(true);
            this->mTiltStdSlider->setVisible(true);
        }
    });

    connect(mRotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if (index == 0)
        {
            this->mRotationAverageSlider->setVisible(false);
            this->mRotationStdSlider->setVisible(false);
        }
        else
        {
            this->mRotationAverageSlider->setVisible(true);
            this->mRotationStdSlider->setVisible(true);
        }
    });
}

void CrystalSettingsWidget::SetInitialValues(HaloSim::CrystalPopulation crystal)
{
    mCaRatioSlider->setValue(crystal.caRatioAverage);
    mCaRatioStdSlider->setValue(crystal.caRatioStd);

    mTiltDistributionComboBox->setCurrentIndex(crystal.tiltDistribution);
    mTiltAverageSlider->setValue(crystal.tiltAverage);
    mTiltStdSlider->setValue(crystal.tiltStd);

    mRotationDistributionComboBox->setCurrentIndex(crystal.rotationDistribution);
    mRotationAverageSlider->setValue(crystal.rotationAverage);
    mRotationStdSlider->setValue(crystal.rotationStd);
}

void CrystalSettingsWidget::setupUi()
{
    auto mainLayout = new QFormLayout(this);

    mCaRatioSlider = new SliderSpinBox();
    mCaRatioSlider->setMinimum(0.0);
    mCaRatioSlider->setMaximum(15.0);

    mCaRatioStdSlider = new SliderSpinBox();
    mCaRatioStdSlider->setMinimum(0.0);
    mCaRatioStdSlider->setMaximum(10.0);

    mTiltDistributionComboBox = new QComboBox();
    mTiltDistributionComboBox->addItems({"Uniform", "Gaussian"});

    mTiltAverageSlider = new SliderSpinBox();
    mTiltAverageSlider->setMinimum(0.0);
    mTiltAverageSlider->setMaximum(180.0);
    mTiltAverageSlider->setSuffix("°");

    mTiltStdSlider = new SliderSpinBox();
    mTiltStdSlider->setMinimum(0.0);
    mTiltStdSlider->setMaximum(360.0);
    mTiltStdSlider->setSuffix("°");

    mRotationDistributionComboBox = new QComboBox();
    mRotationDistributionComboBox->addItems({"Uniform", "Gaussian"});

    mRotationAverageSlider = new SliderSpinBox();
    mRotationAverageSlider->setMinimum(0.0);
    mRotationAverageSlider->setMaximum(180.0);
    mRotationAverageSlider->setSuffix("°");

    mRotationStdSlider = new SliderSpinBox();
    mRotationStdSlider->setMinimum(0.0);
    mRotationStdSlider->setMaximum(360.0);
    mRotationStdSlider->setSuffix("°");

    mainLayout->addRow("C/A ratio average", mCaRatioSlider);
    mainLayout->addRow("C/A ratio std.", mCaRatioStdSlider);

    mainLayout->addRow("Tilt distribution", mTiltDistributionComboBox);
    mainLayout->addRow("Tilt average", mTiltAverageSlider);
    mainLayout->addRow("Tilt std.", mTiltStdSlider);

    mainLayout->addRow("Rotation distribution", mRotationDistributionComboBox);
    mainLayout->addRow("Rotation average", mRotationAverageSlider);
    mainLayout->addRow("Rotation std.", mRotationStdSlider);
}

HaloSim::CrystalPopulation CrystalSettingsWidget::stateToCrystalPopulation() const
{
    HaloSim::CrystalPopulation crystal;
    crystal.caRatioAverage = mCaRatioSlider->value();
    crystal.caRatioStd = mCaRatioStdSlider->value();
    crystal.tiltDistribution = mTiltDistributionComboBox->currentIndex();
    crystal.tiltAverage = mTiltAverageSlider->value();
    crystal.tiltStd = mTiltStdSlider->value();
    crystal.rotationDistribution = mRotationDistributionComboBox->currentIndex();
    crystal.rotationAverage = mRotationAverageSlider->value();
    crystal.rotationStd = mRotationStdSlider->value();

    return crystal;
}
