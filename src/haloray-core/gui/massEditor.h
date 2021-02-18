#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <gui/models/crystalModel.h>

class QTableView;

namespace HaloRay
{

class MassEditor : public QDialog
{
    Q_OBJECT
public:
    MassEditor(CrystalModel *model, QWidget *parent = nullptr);

private:
    QTableView *m_table;

};

}
