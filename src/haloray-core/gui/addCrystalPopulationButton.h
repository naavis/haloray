#pragma once
#include <QToolButton>
#include "../simulation/crystalPopulation.h"

class QWidget;
class QMenu;
class QAction;
class QToolButton;

namespace HaloRay
{

class AddCrystalPopulationButton : public QToolButton
{
    Q_OBJECT
public:
    AddCrystalPopulationButton(QWidget *parent = nullptr);

signals:
    void addPopulation(CrystalPopulationPreset);

private:
    QMenu *m_menu;
    QAction *m_addRandom;
    QAction *m_addPlate;
    QAction *m_addColumn;
    QAction *m_addParry;
    QAction *m_addLowitz;
    QAction *m_addPyramid;
};

}
