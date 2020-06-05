#include "addCrystalPopulationButton.h"
#include <QMenu>
#include <QAction>

namespace HaloRay
{

AddCrystalPopulationButton::AddCrystalPopulationButton(QWidget *parent)
    : QToolButton(parent)
{
    mMenu = new QMenu(this);

    mAddRandom = new QAction(tr("Add random"), this);
    mAddPlate = new QAction(tr("Add plate"), this);
    mAddColumn = new QAction(tr("Add column"), this);
    mAddParry = new QAction(tr("Add Parry"), this);
    mAddLowitz = new QAction(tr("Add Lowitz"), this);

    mMenu->addActions({mAddRandom,
                       mAddPlate,
                       mAddColumn,
                       mAddParry,
                       mAddLowitz});

    setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
    setIcon(QIcon::fromTheme("list-add"));

    setMenu(mMenu);

    connect(this, &AddCrystalPopulationButton::clicked, [this]() { emit addPopulation(HaloRay::CrystalPopulationPreset::Random); });
    connect(mAddRandom, &QAction::triggered, [this]() { emit addPopulation(HaloRay::CrystalPopulationPreset::Random); });
    connect(mAddPlate, &QAction::triggered, [this]() { emit addPopulation(HaloRay::CrystalPopulationPreset::Plate); });
    connect(mAddColumn, &QAction::triggered, [this]() { emit addPopulation(HaloRay::CrystalPopulationPreset::Column); });
    connect(mAddParry, &QAction::triggered, [this]() { emit addPopulation(HaloRay::CrystalPopulationPreset::Parry); });
    connect(mAddLowitz, &QAction::triggered, [this]() { emit addPopulation(HaloRay::CrystalPopulationPreset::Lowitz); });
}

}
