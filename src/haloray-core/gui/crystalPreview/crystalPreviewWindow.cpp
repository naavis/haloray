#include "crystalPreviewWindow.h"
#include "../crystalModel.h"
#include "previewRenderArea.h"
#include <QComboBox>
#include <QVBoxLayout>

namespace HaloRay
{

CrystalPreviewWindow::CrystalPreviewWindow(CrystalModel *model, QWidget *parent)
    : QWidget(parent),
      m_crystalModel(model)
{
    setupUi();

    connect(m_populationSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), m_renderArea, &PreviewRenderArea::onPopulationSelectionChange);
}

void CrystalPreviewWindow::setupUi()
{
    // TODO: Replace minimum size with size policy and size hint
    setMinimumSize(QSize(320, 320));
    setWindowTitle(tr("Crystal preview"));


    m_populationSelector = new QComboBox();
    m_populationSelector->setModel(m_crystalModel);
    m_populationSelector->setModelColumn(CrystalModel::PopulationName);

    m_renderArea = new PreviewRenderArea(m_crystalModel);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_renderArea);
    layout->addWidget(m_populationSelector);
}

}
