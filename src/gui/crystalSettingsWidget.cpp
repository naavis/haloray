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
        mTiltAverageSlider->setVisible(showControls);
        mTiltStdSlider->setVisible(showControls);
        mTiltAverageLabel->setVisible(showControls);
        mTiltStdLabel->setVisible(showControls);
    };
    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), tiltVisibilityHandler);

    auto rotationVisibilityHandler = [this](int index) {
        bool showControls = index != 0;
        mRotationAverageSlider->setVisible(showControls);
        mRotationStdSlider->setVisible(showControls);
        mRotationAverageLabel->setVisible(showControls);
        mRotationStdLabel->setVisible(showControls);
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
        mModel->removeRow(index);
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
