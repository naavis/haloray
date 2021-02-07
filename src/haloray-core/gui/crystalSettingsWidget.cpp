#include "crystalSettingsWidget.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDataWidgetMapper>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "sliderSpinBox.h"
#include "addCrystalPopulationButton.h"
#include "../simulation/crystalPopulation.h"
#include "crystalModel.h"

namespace HaloRay
{

CrystalSettingsWidget::CrystalSettingsWidget(std::shared_ptr<CrystalPopulationRepository> crystalRepository, QWidget *parent)
    : QGroupBox("Crystal population settings", parent),
      m_model(new CrystalModel(crystalRepository)),
      m_nextPopulationId(1)
{
    setupUi();

    auto tiltVisibilityHandler = [this](int index) {
        bool showControls = index != 0;
        setTiltVisibility(showControls);
    };
    connect(m_tiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), tiltVisibilityHandler);

    auto rotationVisibilityHandler = [this](int index) {
        bool showControls = index != 0;
        setRotationVisibility(showControls);
    };
    connect(m_rotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), rotationVisibilityHandler);

    m_mapper = new QDataWidgetMapper();
    m_mapper->setModel(m_model);
    m_mapper->addMapping(m_caRatioSlider, CrystalModel::CaRatioAverage);
    m_mapper->addMapping(m_caRatioStdSlider, CrystalModel::CaRatioStd);
    m_mapper->addMapping(m_tiltDistributionComboBox, CrystalModel::TiltDistribution, "currentIndex");
    m_mapper->addMapping(m_tiltAverageSlider, CrystalModel::TiltAverage);
    m_mapper->addMapping(m_tiltStdSlider, CrystalModel::TiltStd);
    m_mapper->addMapping(m_rotationDistributionComboBox, CrystalModel::RotationDistribution, "currentIndex");
    m_mapper->addMapping(m_rotationAverageSlider, CrystalModel::RotationAverage);
    m_mapper->addMapping(m_rotationStdSlider, CrystalModel::RotationStd);
    m_mapper->addMapping(m_upperApexAngleSpinBox, CrystalModel::UpperApexAngle);
    m_mapper->addMapping(m_upperApexHeightAverageSlider, CrystalModel::UpperApexHeightAverage);
    m_mapper->addMapping(m_upperApexHeightStdSlider, CrystalModel::UpperApexHeightStd);
    m_mapper->addMapping(m_lowerApexAngleSpinBox, CrystalModel::LowerApexAngle);
    m_mapper->addMapping(m_lowerApexHeightAverageSlider, CrystalModel::LowerApexHeightAverage);
    m_mapper->addMapping(m_lowerApexHeightStdSlider, CrystalModel::LowerApexHeightStd);
    m_mapper->addMapping(m_weightSpinBox, CrystalModel::PopulationWeight);
    m_mapper->toFirst();
    m_mapper->setSubmitPolicy(QDataWidgetMapper::SubmitPolicy::AutoSubmit);

    connect(m_model, &CrystalModel::dataChanged, this, &CrystalSettingsWidget::crystalChanged);
    connect(m_model, &CrystalModel::rowsInserted, this, &CrystalSettingsWidget::crystalChanged);
    connect(m_model, &CrystalModel::rowsRemoved, this, &CrystalSettingsWidget::crystalChanged);

    /*
    QDataWidgetMapper only submits data when Enter is pressed, or when a text
    box loses focus, so the sliders and comboboxes must still be manually connected
    to the submit slot of the mapper.
    */
    connect(m_weightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_caRatioSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_caRatioStdSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_tiltDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_tiltAverageSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_tiltStdSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_rotationDistributionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_rotationAverageSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_rotationStdSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_upperApexAngleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_upperApexHeightAverageSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_upperApexHeightStdSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_lowerApexAngleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_lowerApexHeightAverageSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    connect(m_lowerApexHeightStdSlider, &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);

    connect(m_populationComboBox, QOverload<int>::of(&QComboBox::activated), m_mapper, &QDataWidgetMapper::setCurrentIndex);
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged, m_populationComboBox, QOverload<int>::of(&QComboBox::setCurrentIndex));

    tiltVisibilityHandler(m_tiltDistributionComboBox->currentIndex());
    rotationVisibilityHandler(m_rotationDistributionComboBox->currentIndex());

    connect(m_AddPopulationButton, &AddCrystalPopulationButton::addPopulation, [this](CrystalPopulationPreset preset) {
        m_model->addRow(preset);
        addPopulationComboBoxItem();
        m_mapper->toLast();
        updateRemovePopulationButtonState();
    });

    connect(m_removePopulationButton, &QToolButton::clicked, [this]() {
        int index = m_mapper->currentIndex();
        if (index == 0)
            m_mapper->toNext();
        else
            m_mapper->toPrevious();
        bool success = m_model->removeRow(index);
        if (success)
            m_populationComboBox->removeItem(index);

        updateRemovePopulationButtonState();
    });

    connect(m_populationComboBox, &QComboBox::editTextChanged, [this](const QString &text) {
        m_populationComboBox->setItemText(m_populationComboBox->currentIndex(), text);
    });

    fillPopulationComboBox();
    updateRemovePopulationButtonState();
}

void CrystalSettingsWidget::addPopulationComboBoxItem()
{
    m_populationComboBox->addItem(QString("Population %1").arg(m_nextPopulationId++));
}

void CrystalSettingsWidget::fillPopulationComboBox()
{
    m_populationComboBox->addItems({"Columns", "Plates", "Random"});
}

void CrystalSettingsWidget::updateRemovePopulationButtonState()
{
    m_removePopulationButton->setEnabled(m_model->rowCount() > 1);
}

void CrystalSettingsWidget::setupUi()
{
    setMaximumWidth(400);

    m_populationComboBox = new QComboBox();
    m_populationComboBox->setEditable(true);
    m_populationComboBox->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
    m_populationComboBox->setDuplicatesEnabled(true);

    m_AddPopulationButton = new AddCrystalPopulationButton();
    m_AddPopulationButton->setIconSize(QSize(24, 24));

    m_removePopulationButton = new QToolButton();
    m_removePopulationButton->setIcon(QIcon::fromTheme("list-remove"));
    m_removePopulationButton->setIconSize(QSize(24, 24));

    m_caRatioSlider = new SliderSpinBox(0.0, 15.0);

    m_caRatioStdSlider = new SliderSpinBox(0.0, 10.0);

    m_tiltDistributionComboBox = new QComboBox();
    m_tiltDistributionComboBox->addItems({tr("Uniform"), tr("Gaussian")});

    m_tiltAverageLabel = new QLabel(tr("Average"));
    m_tiltAverageSlider = SliderSpinBox::createAngleSlider(0.0, 180.0);

    m_tiltStdLabel = new QLabel(tr("Standard deviation"));
    m_tiltStdSlider = SliderSpinBox::createAngleSlider(0.0, 360.0);

    m_rotationDistributionComboBox = new QComboBox();
    m_rotationDistributionComboBox->addItems({tr("Uniform"), tr("Gaussian")});

    m_rotationAverageLabel = new QLabel(tr("Average"));
    m_rotationAverageSlider = SliderSpinBox::createAngleSlider(0.0, 180.0);

    m_rotationStdLabel = new QLabel(tr("Standard deviation"));
    m_rotationStdSlider = SliderSpinBox::createAngleSlider(0.0, 360.0);

    m_upperApexAngleSpinBox = new QDoubleSpinBox();
    m_upperApexAngleSpinBox->setMinimum(0.0);
    m_upperApexAngleSpinBox->setMaximum(180.0);
    m_upperApexAngleSpinBox->setSuffix("°");

    m_upperApexHeightAverageSlider = new SliderSpinBox(0.0, 1.0);

    m_upperApexHeightStdSlider = new SliderSpinBox(0.0, 5.0);

    m_lowerApexAngleSpinBox = new QDoubleSpinBox();
    m_lowerApexAngleSpinBox->setMinimum(0.0);
    m_lowerApexAngleSpinBox->setMaximum(180.0);
    m_lowerApexAngleSpinBox->setSuffix("°");

    m_lowerApexHeightAverageSlider = new SliderSpinBox(0.0, 1.0);

    m_lowerApexHeightStdSlider = new SliderSpinBox(0.0, 5.0);

    m_weightSpinBox = new QSpinBox();
    m_weightSpinBox->setMinimum(0);
    m_weightSpinBox->setMaximum(10000);

    auto mainLayout = new QFormLayout(this);

    auto populationButtonLayout = new QHBoxLayout();
    populationButtonLayout->addWidget(m_populationComboBox);
    populationButtonLayout->addWidget(m_AddPopulationButton);
    populationButtonLayout->addWidget(m_removePopulationButton);

    mainLayout->addRow(populationButtonLayout);
    mainLayout->addRow(tr("Population weight"), m_weightSpinBox);

    mainLayout->addItem(new QSpacerItem(0, 5));
    mainLayout->addRow(tr("C/A ratio average"), m_caRatioSlider);
    mainLayout->addRow(tr("C/A ratio std."), m_caRatioStdSlider);
    mainLayout->addItem(new QSpacerItem(0, 5));

    auto pyramidGroupBox = new QGroupBox(tr("Pyramid caps"));
    auto pyramidLayout = new QFormLayout(pyramidGroupBox);
    mainLayout->addRow(pyramidGroupBox);
    pyramidLayout->addRow(tr("Upper apex angle"), m_upperApexAngleSpinBox);
    pyramidLayout->addRow(tr("Upper apex average height"), m_upperApexHeightAverageSlider);
    pyramidLayout->addRow(tr("Upper apex height std."), m_upperApexHeightStdSlider);
    pyramidLayout->addItem(new QSpacerItem(0, 5));
    pyramidLayout->addRow(tr("Lower apex angle"), m_lowerApexAngleSpinBox);
    pyramidLayout->addRow(tr("Lower apex average height"), m_lowerApexHeightAverageSlider);
    pyramidLayout->addRow(tr("Lower apex height std."), m_lowerApexHeightStdSlider);

    auto tiltGroupBox = new QGroupBox(tr("C-axis tilt"));
    auto tiltLayout = new QFormLayout(tiltGroupBox);
    mainLayout->addRow(tiltGroupBox);
    tiltLayout->addRow(tr("Distribution"), m_tiltDistributionComboBox);
    tiltLayout->addRow(m_tiltAverageLabel, m_tiltAverageSlider);
    tiltLayout->addRow(m_tiltStdLabel, m_tiltStdSlider);

    auto rotationGroupBox = new QGroupBox(tr("Rotation around C-axis"));
    auto rotationLayout = new QFormLayout(rotationGroupBox);
    mainLayout->addRow(rotationGroupBox);
    rotationLayout->addRow(tr("Distribution"), m_rotationDistributionComboBox);
    rotationLayout->addRow(m_rotationAverageLabel, m_rotationAverageSlider);
    rotationLayout->addRow(m_rotationStdLabel, m_rotationStdSlider);
}

void CrystalSettingsWidget::setTiltVisibility(bool visible)
{
    m_tiltAverageSlider->setVisible(visible);
    m_tiltStdSlider->setVisible(visible);
    m_tiltAverageLabel->setVisible(visible);
    m_tiltStdLabel->setVisible(visible);
}

void CrystalSettingsWidget::setRotationVisibility(bool visible)
{
    m_rotationAverageSlider->setVisible(visible);
    m_rotationStdSlider->setVisible(visible);
    m_rotationAverageLabel->setVisible(visible);
    m_rotationStdLabel->setVisible(visible);
}

}
