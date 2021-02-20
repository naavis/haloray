#pragma once

class QString;

namespace HaloRay
{

class SimulationEngine;
class SimulationStateModel;
class CrystalModel;
class CrystalPopulationRepository;

class StateSaver
{
public:
    static void SaveState(QString filename, SimulationEngine *engine, CrystalPopulationRepository *crystals);
    static void LoadState(QString filename, SimulationStateModel *simState, CrystalModel *crystalModel);
};

}

