#pragma once

#include "Game.h"
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"

using namespace sc2;

bool Army::isStructure() {
    return true;
}