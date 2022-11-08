#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h

using namespace sc2;

class Army : Unit {
    private:
        bool isStructure();

        enum squadron {
            MINERAL_GATHERER,
            VESPENE_GATHERER,
            BUILDER,
            GROUND_SQUADRON_A,
            GROUND_SQUADRON_B,
            GROUND_SQUADRON_C,
            AIR_SQUADRON
        };

        int value; // the cost to build the unit (minerals + vespene)

        ActionRaw currentAction;
};

class GameStatus { // rough pseudocode
    private:
        int value_lost; // sum of the value of all units/structures lost
        int enemy_value_lost; // sum of the value of all enemy units/structures lost
};

class Strategy { // rough pseudocode
    int max_amount;
    int priority_constant;
    int min_minerals;
    int min_gas;
    int min_food;
    int min_food_cap;
    int min_time;
    bool soft_triggers_per_unit;
    Unit unit_type; 
    int each_minerals;
    int each_gas;
    int each_food;
    int each_food_cap;
    int each_time;
    int min_minerals;
    int min_gas;
    int min_food;
    int min_food_cap;
    int min_time;
    bool attacked;
    bool attacking;
};
