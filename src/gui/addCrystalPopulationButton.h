#pragma once
#include <QToolButton>
#include <QMenu>
#include <QWidget>
#include <QAction>
#include "../simulation/crystalPopulation.h"

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
