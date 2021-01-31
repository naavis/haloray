#include "addCrystalPopulationButton.h"
#include <QMenu>
#include <QAction>

namespace HaloRay
{

AddCrystalPopulationButton::AddCrystalPopulationButton(QWidget *parent)
    : QToolButton(parent)
{
    m_menu = new QMenu(this);

    m_addRandom = new QAction(tr("Add random"), this);
    m_addPlate = new QAction(tr("Add plate"), this);
    m_addColumn = new QAction(tr("Add column"), this);
    m_addParry = new QAction(tr("Add Parry"), this);
    m_addLowitz = new QAction(tr("Add Lowitz"), this);

    m_menu->addActions({m_addRandom,
                       m_addPlate,
                       m_addColumn,
                       m_addParry,
                       m_addLowitz});

    setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
    setIcon(QIcon::fromTheme("list-add"));

    setMenu(m_menu);

    connect(this, &AddCrystalPopulationButton::clicked, [this]() { emit addPopulation(CrystalPopulationPreset::Random); });
    connect(m_addRandom, &QAction::triggered, [this]() { emit addPopulation(CrystalPopulationPreset::Random); });
    connect(m_addPlate, &QAction::triggered, [this]() { emit addPopulation(CrystalPopulationPreset::Plate); });
    connect(m_addColumn, &QAction::triggered, [this]() { emit addPopulation(CrystalPopulationPreset::Column); });
    connect(m_addParry, &QAction::triggered, [this]() { emit addPopulation(CrystalPopulationPreset::Parry); });
    connect(m_addLowitz, &QAction::triggered, [this]() { emit addPopulation(CrystalPopulationPreset::Lowitz); });
}

}
