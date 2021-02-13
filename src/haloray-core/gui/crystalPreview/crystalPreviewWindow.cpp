#include "crystalPreviewWindow.h"
#include "../crystalModel.h"
#include <QComboBox>

namespace HaloRay
{

CrystalPreviewWindow::CrystalPreviewWindow(CrystalModel *model, QWidget *parent)
    : QWidget(parent),
      m_crystalModel(model)
{
    setupUi();
}

void CrystalPreviewWindow::setupUi()
{
    setMinimumSize(QSize(500, 500));
    setWindowTitle(tr("Crystal preview"));

    m_populationSelector = new QComboBox(this);
    m_populationSelector->setModel(m_crystalModel);
    m_populationSelector->setModelColumn(CrystalModel::PopulationName);
}

}
