#pragma once

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"

using namespace sc2;

class Army : Unit {
private:
    bool isStructure();
    enum mobsron {
        MINERAL_GATHERER,
        VESPENE_GATHERER,
        BUILDER,
        GROUND_MOBRON_A,
        GROUND_MOBRON_B,
        GROUND_MOBRON_C,
        AIR_MOBRON
    };
    int value; // the cost to build the unit (minerals + vespene)
    ActionRaw currentAction;
};

class GameStatus { // rough pseudocode
private:
    int value_lost; // sum of the value of all units/structures lost
    int enemy_value_lost; // sum of the value of all enemy units/structures lost
};