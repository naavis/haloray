#include "crystalSettingsWidget.h"
#include <QFormLayout>
#include "../simulation/crystalPopulation.h"

CrystalSettingsWidget::CrystalSettingsWidget(QWidget *parent)
    : QGroupBox("Crystal settings", parent)
{
    setupUi();

    auto crystalChangeHandler = [this]() {
        crystalChanged(stateToCrystalPopulation());
    };

    connect(mCaRatioSlider, &SliderSpinBox::valueChanged, crystalChangeHandler);
    connect(mCaRatioStdSlider, &SliderSpinBox::valueChanged, crystalChangeHandler);

    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), crystalChangeHandler);
    connect(mTiltAverageSlider, &SliderSpinBox::valueChanged, crystalChangeHandler);
    connect(mTiltStdSlider, &SliderSpinBox::valueChanged, crystalChangeHandler);

    connect(mRotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), crystalChangeHandler);
    connect(mRotationAverageSlider, &SliderSpinBox::valueChanged, crystalChangeHandler);
    connect(mRotationStdSlider, &SliderSpinBox::valueChanged, crystalChangeHandler);

    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        bool showControls = index != 0;
        mTiltAverageSlider->setVisible(showControls);
        mTiltStdSlider->setVisible(showControls);
        mTiltAverageLabel->setVisible(showControls);
        mTiltStdLabel->setVisible(showControls);
    });

    connect(mRotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        bool showControls = index != 0;
        mRotationAverageSlider->setVisible(showControls);
        mRotationStdSlider->setVisible(showControls);
        mRotationAverageLabel->setVisible(showControls);
        mRotationStdLabel->setVisible(showControls);
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

    mTiltAverageLabel = new QLabel("Average");

    mTiltStdSlider = new SliderSpinBox();
    mTiltStdSlider->setMinimum(0.0);
    mTiltStdSlider->setMaximum(360.0);
    mTiltStdSlider->setSuffix("°");

    mTiltStdLabel = new QLabel("Standard deviation");

    mRotationDistributionComboBox = new QComboBox();
    mRotationDistributionComboBox->addItems({"Uniform", "Gaussian"});

    mRotationAverageSlider = new SliderSpinBox();
    mRotationAverageSlider->setMinimum(0.0);
    mRotationAverageSlider->setMaximum(180.0);
    mRotationAverageSlider->setSuffix("°");

    mRotationAverageLabel = new QLabel("Average");

    mRotationStdSlider = new SliderSpinBox();
    mRotationStdSlider->setMinimum(0.0);
    mRotationStdSlider->setMaximum(360.0);
    mRotationStdSlider->setSuffix("°");

    mRotationStdLabel = new QLabel("Standard deviation");

    auto mainLayout = new QFormLayout(this);

    mainLayout->addRow("C/A ratio average", mCaRatioSlider);
    mainLayout->addRow("C/A ratio std.", mCaRatioStdSlider);

    auto tiltGroupBox = new QGroupBox("C-axis tilt");
    auto tiltLayout = new QFormLayout(tiltGroupBox);
    mainLayout->addRow(tiltGroupBox);
    tiltLayout->addRow("Distribution", mTiltDistributionComboBox);
    tiltLayout->addRow(mTiltAverageLabel, mTiltAverageSlider);
    tiltLayout->addRow(mTiltStdLabel, mTiltStdSlider);

    auto rotationGroupBox = new QGroupBox("Rotation around C-axis");
    auto rotationLayout = new QFormLayout(rotationGroupBox);
    mainLayout->addRow(rotationGroupBox);
    rotationLayout->addRow("Distribution", mRotationDistributionComboBox);
    rotationLayout->addRow(mRotationAverageLabel, mRotationAverageSlider);
    rotationLayout->addRow(mRotationStdLabel, mRotationStdSlider);
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
