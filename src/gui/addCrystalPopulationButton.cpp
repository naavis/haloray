#include "addCrystalPopulationButton.h"

AddCrystalPopulationButton::AddCrystalPopulationButton(QWidget *parent)
    : QToolButton(parent)
{
    //setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    mMenu = new QMenu(this);

    mAddRandom = new QAction("Add random", this);
    mAddPlate = new QAction("Add plate", this);
    mAddColumn = new QAction("Add column", this);
    mAddParry = new QAction("Add Parry", this);
    mAddLowitz = new QAction("Add Lowitz", this);

    mMenu->addActions({mAddRandom,
                       mAddPlate,
                       mAddColumn,
                       mAddParry,
                       mAddLowitz});

    setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
    setIcon(QIcon::fromTheme("list-add"));

    setMenu(mMenu);

    connect(this, &AddCrystalPopulationButton::clicked, [this]() { emit addPopulation(HaloSim::CrystalPopulationPreset::Random); });
    connect(mAddRandom, &QAction::triggered, [this]() { emit addPopulation(HaloSim::CrystalPopulationPreset::Random); });
    connect(mAddPlate, &QAction::triggered, [this]() { emit addPopulation(HaloSim::CrystalPopulationPreset::Plate); });
    connect(mAddColumn, &QAction::triggered, [this]() { emit addPopulation(HaloSim::CrystalPopulationPreset::Column); });
    connect(mAddParry, &QAction::triggered, [this]() { emit addPopulation(HaloSim::CrystalPopulationPreset::Parry); });
    connect(mAddLowitz, &QAction::triggered, [this]() { emit addPopulation(HaloSim::CrystalPopulationPreset::Lowitz); });
}
