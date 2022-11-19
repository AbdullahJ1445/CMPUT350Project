#pragma once
#include "sc2api/sc2_api.h"
#include "Agents.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"
#include "Base.h"

class BotAgent;

class MapChunk {
public:
    MapChunk(BotAgent* agent_, sc2::Point2D location_, bool pathable_);
    uint32_t seen_at();
    bool wasSeen();
    bool isPathable();
    bool inVision(const sc2::ObservationInterface* obs);
    void checkVision(const sc2::ObservationInterface* obs);
    sc2::Point2D getLocation();

private:
    bool pathable;
    BotAgent* agent;
    sc2::Point2D location;
    uint32_t last_seen;
    sc2::Visibility last_visibility;
};

class LocationHandler {
public:
    LocationHandler(BotAgent* agent_);
    sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
    int getIndexOfClosestBase(sc2::Point2D location_);
    void scanChunks(const sc2::ObservationInterface* obs);
    const sc2::Unit* getNearestMineralPatch(sc2::Point2D location);
    const sc2::Unit* getNearestGeyser(sc2::Point2D location);
    const sc2::Unit* getNearestGasStructure(sc2::Point2D location);
    const sc2::Unit*  getNearestTownhall(const sc2::Point2D location);
    sc2::Point2D getOldestLocation(bool pathable_);
    int getPlayerIDForMap(int map_index, sc2::Point2D location);
    void initLocations(int map_index, int p_id);
    void setEnemyStartLocation(sc2::Point2D location_);
    void setProxyLocation(sc2::Point2D);
    sc2::Point2D getEnemyStartLocationByIndex(int index_);
    sc2::Point2D getBestEnemyLocation();
    sc2::Point2D getProxyLocation();
    sc2::Point2D getStartLocation();
    std::vector<Base> bases;

private:
    void initSetStartLocation();
    void initAddEnemyStartLocation(sc2::Point2D location_);
    void initMapChunks();
    sc2::Point2D getClosestUnseenLocation(bool pathable_);

    BotAgent* agent;
    std::vector<MapChunk> map_chunks;
    std::vector<sc2::Point2D> enemy_start_locations;
    sc2::Point2D enemy_start_location;
    sc2::Point2D proxy_location;
    sc2::Point2D start_location;
    int enemy_start_location_index;
    
};