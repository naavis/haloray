#include "crystalPreviewWindow.h"
#include "gui/models/crystalModel.h"
#include "previewRenderArea.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QCheckBox>

namespace HaloRay
{

CrystalPreviewWindow::CrystalPreviewWindow(CrystalModel *model, int initialIndex, QWidget *parent)
    : QWidget(parent, Qt::Window),
      m_crystalModel(model),
      m_selectedPopulationIndex(initialIndex)
{
    setupUi();

    connect(m_populationSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), m_renderArea, &PreviewRenderArea::onPopulationSelectionChange);
    connect(m_syncCheckBox, &QCheckBox::toggled, m_populationSelector, &QComboBox::setDisabled);
    connect(m_syncCheckBox, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            m_populationSelector->setCurrentIndex(m_selectedPopulationIndex);
        }
    });
    m_syncCheckBox->setChecked(true);
}

void CrystalPreviewWindow::onMainWindowPopulationSelectionChange(int index)
{
    m_selectedPopulationIndex = index;
    if (m_syncCheckBox->isChecked()) {
        m_populationSelector->setCurrentIndex(index);
    }
}

void CrystalPreviewWindow::setupUi()
{
    setWindowTitle(tr("Crystal preview"));

    m_populationSelector = new QComboBox();
    m_populationSelector->setModel(m_crystalModel);
    m_populationSelector->setModelColumn(CrystalModel::PopulationName);

    m_syncCheckBox = new QCheckBox(tr("Sync with main window"));

    m_renderArea = new PreviewRenderArea(m_crystalModel);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_renderArea);
    layout->addWidget(m_syncCheckBox);
    layout->addWidget(m_populationSelector);
}

}
