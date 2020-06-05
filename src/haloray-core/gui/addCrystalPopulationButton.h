#pragma once
#include <QToolButton>
#include "../simulation/crystalPopulation.h"

class QWidget;
class QMenu;
class QAction;
class QToolButton;

class AddCrystalPopulationButton : public QToolButton
{
    Q_OBJECT
public:
    AddCrystalPopulationButton(QWidget *parent = nullptr);

signals:
    void addPopulation(HaloSim::CrystalPopulationPreset);

private:
    QMenu *mMenu;
    QAction *mAddRandom;
    QAction *mAddPlate;
    QAction *mAddColumn;
    QAction *mAddParry;
    QAction *mAddLowitz;
};
