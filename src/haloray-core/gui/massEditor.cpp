#include "massEditor.h"
#include <QTableView>
#include <QVBoxLayout>

namespace HaloRay
{

MassEditor::MassEditor(CrystalModel *model, QWidget *parent)
  : QDialog(parent)
{
    setWindowTitle(tr("Mass editor"));

    auto layout = new QVBoxLayout(this);
    m_table = new QTableView();
    m_table->setModel(model);
    layout->addWidget(m_table);
}

}
