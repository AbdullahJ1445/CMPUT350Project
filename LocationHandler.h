#pragma once
#include "sc2api/sc2_api.h"
#include "Agents.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"
#include "sc2api/sc2_unit_types.h"

class BotAgent;
class LocationHandler {
public:
    LocationHandler(BotAgent* agent);
    sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
    int getIndexOfClosestBase(sc2::Point2D location_);
    const sc2::Unit* getNearestMineralPatch(sc2::Point2D location);
    const sc2::Unit* getNearestGeyser(sc2::Point2D location);
    const sc2::Unit* getNearestGasStructure(sc2::Point2D location);
    const sc2::Unit*  getNearestTownhall(const sc2::Point2D location);
    int getPlayerIDForMap(int map_index, sc2::Point2D location);
    void initLocations(int map_index, int p_id);

private:
    BotAgent* agent;
};