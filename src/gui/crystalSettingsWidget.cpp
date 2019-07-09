#include "crystalSettingsWidget.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include "../simulation/crystalPopulation.h"

CrystalSettingsWidget::CrystalSettingsWidget(std::shared_ptr<HaloSim::CrystalPopulationRepository> crystalRepository, QWidget *parent)
    : QGroupBox("Crystal settings", parent),
      mModel(new CrystalModel(crystalRepository)),
      mNextPopulationId(1)
{
    setupUi();

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
    mMapper->addMapping(mWeightSpinBox, 8);
    mMapper->toFirst();
    mMapper->setSubmitPolicy(QDataWidgetMapper::SubmitPolicy::AutoSubmit);

    connect(mModel, &CrystalModel::dataChanged, this, &CrystalSettingsWidget::crystalChanged);

    /*
    QDataWidgetMapper only submits data when Enter is pressed, or when a text
    box loses focus, so the sliders and comboboxes must still be manually connected
    to the submit slot of the mapper.
    */
    connect(mWeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), mMapper, &QDataWidgetMapper::submit);
    connect(mCaRatioSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mCaRatioStdSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mTiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), mMapper, &QDataWidgetMapper::submit);
    connect(mTiltAverageSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mTiltStdSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mRotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), mMapper, &QDataWidgetMapper::submit);
    connect(mRotationAverageSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);
    connect(mRotationStdSlider, &SliderSpinBox::valueChanged, mMapper, &QDataWidgetMapper::submit);

    connect(mPopulationComboBox, QOverload<int>::of(&QComboBox::activated), mMapper, &QDataWidgetMapper::setCurrentIndex);
    connect(mMapper, &QDataWidgetMapper::currentIndexChanged, mPopulationComboBox, QOverload<int>::of(&QComboBox::setCurrentIndex));

    tiltVisibilityHandler(mTiltDistributionComboBox->currentIndex());
    rotationVisibilityHandler(mRotationDistributionComboBox->currentIndex());

    connect(mAddPopulationButton, &AddCrystalPopulationButton::addPopulation, [this](HaloSim::CrystalPopulationPreset preset) {
        mModel->addRow(preset);
        addPopulationComboBoxItem();
        mMapper->toLast();
        updateRemovePopulationButtonState();
    });

    connect(mRemovePopulationButton, &QPushButton::clicked, [this]() {
        int index = mMapper->currentIndex();
        if (index == 0)
            mMapper->toNext();
        else
            mMapper->toPrevious();
        bool success = mModel->removeRow(index);
        if (success)
            mPopulationComboBox->removeItem(index);

        updateRemovePopulationButtonState();
    });

    connect(mPopulationComboBox, &QComboBox::editTextChanged, [this](const QString &text) {
        mPopulationComboBox->setItemText(mPopulationComboBox->currentIndex(), text);
    });

    fillPopulationComboBox();
    updateRemovePopulationButtonState();
}

void CrystalSettingsWidget::addPopulationComboBoxItem()
{
    mPopulationComboBox->addItem(QString("Population %1").arg(mNextPopulationId++));
}

void CrystalSettingsWidget::fillPopulationComboBox()
{
    mPopulationComboBox->addItems({"Columns", "Plates", "Random"});
}

void CrystalSettingsWidget::updateRemovePopulationButtonState()
{
    mRemovePopulationButton->setEnabled(mModel->rowCount() > 1);
}

void CrystalSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    mPopulationComboBox = new QComboBox();
    mPopulationComboBox->setEditable(true);
    mPopulationComboBox->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
    mPopulationComboBox->setDuplicatesEnabled(true);

    mAddPopulationButton = new AddCrystalPopulationButton();
    mAddPopulationButton->setMinimumHeight(30);

    mRemovePopulationButton = new QPushButton("Remove population");
    mRemovePopulationButton->setMinimumHeight(30);
    mRemovePopulationButton->setStyleSheet("padding: 10px;");

    mCaRatioSlider = new SliderSpinBox(0.0, 15.0);

    mCaRatioStdSlider = new SliderSpinBox(0.0, 10.0);

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

    mWeightSpinBox = new QSpinBox();
    mWeightSpinBox->setMinimum(0);
    mWeightSpinBox->setMaximum(10000);

    auto mainLayout = new QFormLayout(this);
    mainLayout->addRow("Crystal population", mPopulationComboBox);

    auto populationButtonLayout = new QHBoxLayout();
    populationButtonLayout->addWidget(mAddPopulationButton);
    populationButtonLayout->addWidget(mRemovePopulationButton);

    mainLayout->addRow(populationButtonLayout);
    mainLayout->addRow("Population weight", mWeightSpinBox);
    mainLayout->addItem(new QSpacerItem(0, 10));
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
    auto slider = new SliderSpinBox(min, max);
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
