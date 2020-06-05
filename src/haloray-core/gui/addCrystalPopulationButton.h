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
    void addPopulation(HaloRay::CrystalPopulationPreset);

private:
    QMenu *mMenu;
    QAction *mAddRandom;
    QAction *mAddPlate;
    QAction *mAddColumn;
    QAction *mAddParry;
    QAction *mAddLowitz;
};

}
