#include "crystalSettingsWidget.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDataWidgetMapper>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QCheckBox>
#include "components/sliderSpinBox.h"
#include "components/addCrystalPopulationButton.h"
#include "simulation/crystalPopulation.h"
#include "models/crystalModel.h"

namespace HaloRay
{

CrystalSettingsWidget::CrystalSettingsWidget(CrystalModel *model, QWidget *parent)
    : CollapsibleBox("Crystal population settings", false, parent),
      m_model(model)
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

    m_mapper = new QDataWidgetMapper(this);
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
    m_mapper->addMapping(m_populationEnabledCheckBox, CrystalModel::Enabled);
    m_mapper->addMapping(m_populationComboBox, CrystalModel::PopulationName, "currentText");
    for (auto i = 0; i < 6; ++i)
    {
        m_mapper->addMapping(m_prismFaceDistanceSliders[i], CrystalModel::PrismFaceDistance1 + i);
    }
    m_mapper->toFirst();
    m_mapper->setSubmitPolicy(QDataWidgetMapper::SubmitPolicy::AutoSubmit);

    /*
    QDataWidgetMapper only submits data when Enter is pressed, or when a text
    box loses focus, so the sliders and comboboxes must still be manually connected
    to the submit slot of the mapper.
    */
    connect(m_populationEnabledCheckBox, &QCheckBox::toggled, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
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
    for (auto i = 0; i < 6; ++i)
    {
        connect(m_prismFaceDistanceSliders[i], &SliderSpinBox::valueChanged, m_mapper, &QDataWidgetMapper::submit, Qt::QueuedConnection);
    }

    setupPopulationComboBoxConnections();

    auto updateRemovePopulationButtonState = [this]()
    {
        this->m_removePopulationButton->setEnabled(m_model->rowCount() > 1);
    };

    /* These connections are necessary so button state updates if data in the model changes
     * on its own, e.g. during loading saved simulation state. */
    connect(m_model, &CrystalModel::dataChanged, updateRemovePopulationButtonState);
    connect(m_model, &CrystalModel::rowsInserted, updateRemovePopulationButtonState);
    connect(m_model, &CrystalModel::rowsRemoved, updateRemovePopulationButtonState);

    connect(m_model, &CrystalModel::rowsInserted, [this]() {
        if (m_mapper->currentIndex() == -1)
            m_mapper->toFirst();
    });

    connect(m_populationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        emit populationSelectionChanged(index);
    });

    connect(m_addPopulationButton, &AddCrystalPopulationButton::addPopulation, [this, updateRemovePopulationButtonState](CrystalPopulationPreset preset) {
        m_model->addRow(preset);
        m_mapper->toLast();
        updateRemovePopulationButtonState();
    });

    connect(m_removePopulationButton, &QToolButton::clicked, [this, updateRemovePopulationButtonState]() {
        int index = m_mapper->currentIndex();
        m_model->removeRow(index);
        if (index > 1)
        {
            m_mapper->setCurrentIndex(index - 1);
        } else {
            m_mapper->toFirst();
        }

        updateRemovePopulationButtonState();
    });

    tiltVisibilityHandler(m_tiltDistributionComboBox->currentIndex());
    rotationVisibilityHandler(m_rotationDistributionComboBox->currentIndex());
    updateRemovePopulationButtonState();
}

int CrystalSettingsWidget::getCurrentPopulationIndex() const
{
    return m_populationComboBox->currentIndex();
}

void CrystalSettingsWidget::setupUi()
{
    setMaximumWidth(400);
    setMinimumWidth(350);

    m_populationComboBox = new QComboBox();
    m_populationComboBox->setEditable(true);
    m_populationComboBox->setInsertPolicy(QComboBox::InsertPolicy::NoInsert);
    m_populationComboBox->setDuplicatesEnabled(true);

    m_addPopulationButton = new AddCrystalPopulationButton();
    m_addPopulationButton->setIconSize(QSize(24, 24));

    m_removePopulationButton = new QToolButton();
    m_removePopulationButton->setIcon(QIcon::fromTheme("list-remove"));
    m_removePopulationButton->setIconSize(QSize(24, 24));

    m_populationEnabledCheckBox = new QCheckBox();

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

    for (auto i = 0; i < 6; ++i)
    {
        m_prismFaceDistanceSliders[i] = new SliderSpinBox(0.0, 4.0);
    }

    auto tabWidget = new QTabWidget();

    auto mainLayout = new QVBoxLayout(this->contentWidget());

    auto populationManagementLayout = new QHBoxLayout();
    populationManagementLayout->addWidget(m_populationComboBox);
    populationManagementLayout->addWidget(m_addPopulationButton);
    populationManagementLayout->addWidget(m_removePopulationButton);

    QWidget *populationSettingsWidget = new QWidget();
    auto populationSettingsLayout = new QFormLayout(populationSettingsWidget);
    populationSettingsLayout->addRow(populationManagementLayout);
    populationSettingsLayout->addRow(tr("Population enabled"), m_populationEnabledCheckBox);
    populationSettingsLayout->addRow(tr("Population weight"), m_weightSpinBox);
    mainLayout->addWidget(populationSettingsWidget);

    mainLayout->addWidget(tabWidget);

    QWidget *basicShapeTab = new QWidget();
    auto basicShapeLayout = new QFormLayout(basicShapeTab);
    basicShapeLayout->addRow(tr("C/A ratio average"), m_caRatioSlider);
    basicShapeLayout->addRow(tr("C/A ratio std."), m_caRatioStdSlider);
    basicShapeLayout->addItem(new QSpacerItem(0, 5));

    auto tiltGroupBox = new QGroupBox(tr("C-axis tilt"));
    auto tiltLayout = new QFormLayout(tiltGroupBox);
    basicShapeLayout->addRow(tiltGroupBox);
    tiltLayout->addRow(tr("Distribution"), m_tiltDistributionComboBox);
    tiltLayout->addRow(m_tiltAverageLabel, m_tiltAverageSlider);
    tiltLayout->addRow(m_tiltStdLabel, m_tiltStdSlider);

    auto rotationGroupBox = new QGroupBox(tr("Rotation around C-axis"));
    auto rotationLayout = new QFormLayout(rotationGroupBox);
    basicShapeLayout->addRow(rotationGroupBox);
    rotationLayout->addRow(tr("Distribution"), m_rotationDistributionComboBox);
    rotationLayout->addRow(m_rotationAverageLabel, m_rotationAverageSlider);
    rotationLayout->addRow(m_rotationStdLabel, m_rotationStdSlider);

    tabWidget->addTab(basicShapeTab, tr("Basic shape"));

    QWidget *pyramidShapeTab = new QWidget();
    auto pyramidShapeLayout = new QFormLayout(pyramidShapeTab);

    auto pyramidGroupBox = new QGroupBox(tr("Pyramid caps"));
    auto pyramidLayout = new QFormLayout(pyramidGroupBox);
    pyramidShapeLayout->addRow(pyramidGroupBox);
    pyramidLayout->addRow(tr("Upper apex angle"), m_upperApexAngleSpinBox);
    pyramidLayout->addRow(tr("Upper apex average height"), m_upperApexHeightAverageSlider);
    pyramidLayout->addRow(tr("Upper apex height std."), m_upperApexHeightStdSlider);
    pyramidLayout->addItem(new QSpacerItem(0, 5));
    pyramidLayout->addRow(tr("Lower apex angle"), m_lowerApexAngleSpinBox);
    pyramidLayout->addRow(tr("Lower apex average height"), m_lowerApexHeightAverageSlider);
    pyramidLayout->addRow(tr("Lower apex height std."), m_lowerApexHeightStdSlider);

    tabWidget->addTab(pyramidShapeTab, tr("Pyramid caps"));

    QWidget *prismDistanceTab = new QWidget();
    auto prismDistanceLayout = new QFormLayout(prismDistanceTab);
    auto prismFaceDistanceGroupBox = new QGroupBox(tr("Prism face distances from C-axis"));
    auto prismFaceDistanceLayout = new QFormLayout(prismFaceDistanceGroupBox);
    prismDistanceLayout->addRow(prismFaceDistanceGroupBox);
    for (auto i = 0; i < 6; ++i)
    {
        auto labelString = QString("Face %1").arg(i + 3);
        auto labelStdString = labelString.toStdString();
        auto labelCString = labelStdString.c_str();
        prismFaceDistanceLayout->addRow(tr(labelCString), m_prismFaceDistanceSliders[i]);
    }

    tabWidget->addTab(prismDistanceTab, tr("Prism faces"));
}

void CrystalSettingsWidget::setupPopulationComboBoxConnections()
{
    m_populationComboBox->setModel(m_model);
    m_populationComboBox->setModelColumn(CrystalModel::PopulationName);
    m_populationComboBox->setCurrentIndex(m_mapper->currentIndex());

    /* Without this connection the editable QComboBox won't submit changes to
     * underlying model without losing focus or the user pressing Enter */
    connect(m_populationComboBox->lineEdit(), &QLineEdit::textEdited, [this](const QString &text) {
        m_populationComboBox->setItemData(m_populationComboBox->currentIndex(), text, Qt::EditRole);
    });

    /* These connections keep the population combobox selection and the QDataWidgetMapper
     * current index in sync */
    connect(m_populationComboBox, QOverload<int>::of(&QComboBox::activated), m_mapper, &QDataWidgetMapper::setCurrentIndex, Qt::QueuedConnection);
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged, m_populationComboBox, QOverload<int>::of(&QComboBox::setCurrentIndex), Qt::QueuedConnection);
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
