#pragma once

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"

using namespace sc2;

class GameStatus { // rough pseudocode
private:
    int value_lost; // sum of the value of all units/structures lost
    int enemy_value_lost; // sum of the value of all enemy units/structures lost
};