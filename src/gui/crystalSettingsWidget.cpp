#include "crystalSettingsWidget.h"
#include <QFormLayout>
#include "../simulation/crystalPopulation.h"

CrystalSettingsWidget::CrystalSettingsWidget(std::shared_ptr<HaloSim::CrystalPopulationRepository> crystalRepository, QWidget *parent)
    : QGroupBox("Crystal settings", parent),
      mModel(new CrystalModel(crystalRepository))
{
    setupUi();

    mMapper = new QDataWidgetMapper();
    mMapper->setModel(mModel);
    mMapper->addMapping(mCaRatioSlider, 0);
    mMapper->addMapping(mCaRatioStdSlider, 1);
    mMapper->addMapping(mTiltDistributionComboBox, 2, "currentIndex");
    mMapper->addMapping(mTiltAverageSlider, 3);
    mMapper->addMapping(mTiltStdSlider, 4);
    mMapper->addMapping(mRotationDistributionComboBox, 5, "currentIndex");
    mMapper->addMapping(mRotationAverageSlider, 6);
    mMapper->addMapping(mRotationStdSlider, 7);
    mMapper->toFirst();
    mMapper->setSubmitPolicy(QDataWidgetMapper::SubmitPolicy::AutoSubmit);

    connect(mModel, &CrystalModel::dataChanged, this, &CrystalSettingsWidget::crystalChanged);

    auto tiltVisibilityHandler = [this](int index) {
        bool showControls = index != 0;
        setTiltVisibility(showControls);
    };
    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), tiltVisibilityHandler);

    auto rotationVisibilityHandler = [this](int index) {
        bool showControls = index != 0;
        setRotationVisibility(showControls);
    };
    connect(mRotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), rotationVisibilityHandler);

    connect(mCaRatioSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mCaRatioStdSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), mMapper, &QDataWidgetMapper::submit);
    connect(mTiltAverageSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mTiltStdSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mRotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), mMapper, &QDataWidgetMapper::submit);
    connect(mRotationAverageSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mRotationStdSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);

    connect(mPopulationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), mMapper, &QDataWidgetMapper::setCurrentIndex);
    connect(mMapper, &QDataWidgetMapper::currentIndexChanged, mPopulationComboBox, QOverload<int>::of(&QComboBox::setCurrentIndex));

    tiltVisibilityHandler(mTiltDistributionComboBox->currentIndex());
    rotationVisibilityHandler(mRotationDistributionComboBox->currentIndex());

    connect(mAddPopulationButton, &QPushButton::clicked, [this]() {
        mModel->addRow();
        mPopulationComboBox->addItem(QString("Population %1").arg(mModel->rowCount() - 1));
        mMapper->toLast();
    });
    connect(mRemovePopulationButton, &QPushButton::clicked, [this]() {
        int index = mMapper->currentIndex();
        bool success = mModel->removeRow(index);
        if (success)
            mPopulationComboBox->removeItem(index);
    });

    for (auto i = 0; i < mModel->rowCount(); ++i)
    {
        mPopulationComboBox->addItem(QString("Population %1").arg(i));
    }
}

void CrystalSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    mPopulationComboBox = new QComboBox();
    mAddPopulationButton = new QPushButton("Add");
    mRemovePopulationButton = new QPushButton("Remove");

    mCaRatioSlider = new SliderSpinBox();
    mCaRatioSlider->setMinimum(0.0);
    mCaRatioSlider->setMaximum(15.0);

    mCaRatioStdSlider = new SliderSpinBox();
    mCaRatioStdSlider->setMinimum(0.0);
    mCaRatioStdSlider->setMaximum(10.0);

    mTiltDistributionComboBox = new QComboBox();
    mTiltDistributionComboBox->addItems({"Uniform", "Gaussian"});

    mTiltAverageLabel = new QLabel("Average");
    mTiltAverageSlider = createAngleSlider(0.0, 180.0);

    mTiltStdLabel = new QLabel("Standard deviation");
    mTiltStdSlider = createAngleSlider(0.0, 360.0);

    mRotationDistributionComboBox = new QComboBox();
    mRotationDistributionComboBox->addItems({"Uniform", "Gaussian"});

    mRotationAverageLabel = new QLabel("Average");
    mRotationAverageSlider = createAngleSlider(0.0, 180.0);

    mRotationStdLabel = new QLabel("Standard deviation");
    mRotationStdSlider = createAngleSlider(0.0, 360.0);

    auto mainLayout = new QFormLayout(this);
    mainLayout->addRow("Population", mPopulationComboBox);
    mainLayout->addRow(mAddPopulationButton);
    mainLayout->addRow(mRemovePopulationButton);

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

SliderSpinBox *CrystalSettingsWidget::createAngleSlider(double min, double max)
{
    auto slider = new SliderSpinBox();
    slider->setMinimum(0.0);
    slider->setMaximum(180.0);
    slider->setSuffix("°");
    return slider;
}

void CrystalSettingsWidget::setTiltVisibility(bool visible)
{
    mTiltAverageSlider->setVisible(visible);
    mTiltStdSlider->setVisible(visible);
    mTiltAverageLabel->setVisible(visible);
    mTiltStdLabel->setVisible(visible);
}

void CrystalSettingsWidget::setRotationVisibility(bool visible)
{
    mRotationAverageSlider->setVisible(visible);
    mRotationStdSlider->setVisible(visible);
    mRotationAverageLabel->setVisible(visible);
    mRotationStdLabel->setVisible(visible);
}
