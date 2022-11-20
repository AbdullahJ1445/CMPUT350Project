#pragma once
#include "sc2api/sc2_api.h"
#include "BasicSc2Bot.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"
#include "Base.h"

# define NO_POINT_FOUND sc2::Point2D(-2.5252, -2.5252)

class BasicSc2Bot;

class MapChunk {
public:
    MapChunk(BasicSc2Bot* agent_, sc2::Point2D location_, bool pathable_);
    int seen_at();
    bool wasSeen();
    size_t getID();
    bool isPathable();
    bool inVision(const sc2::ObservationInterface* obs);
    void checkVision(const sc2::ObservationInterface* obs);
    sc2::Point2D getLocation();

private:
    bool pathable;
    BasicSc2Bot* agent;
    sc2::Point2D location;
    int last_seen;
    size_t id;
    sc2::Visibility last_visibility;
};

class LocationHandler {
public:
    LocationHandler(BasicSc2Bot* agent_);
    sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
    int getIndexOfClosestBase(sc2::Point2D location_);
    void scanChunks(const sc2::ObservationInterface* obs);
    const sc2::Unit* getNearestMineralPatch(sc2::Point2D location);
    const sc2::Unit* getNearestGeyser(sc2::Point2D location);
    const sc2::Unit* getNearestGasStructure(sc2::Point2D location);
    const sc2::Unit*  getNearestTownhall(const sc2::Point2D location);
    sc2::Point2D getOldestLocation(bool pathable_=true);
    int getPlayerIDForMap(int map_index, sc2::Point2D location);
    void initLocations(int map_index, int p_id);
    void setEnemyStartLocation(sc2::Point2D location_);
    void setProxyLocation(sc2::Point2D);
    sc2::Point2D getEnemyStartLocationByIndex(int index_);
    sc2::Point2D getBestEnemyLocation();
    sc2::Point2D getProxyLocation();
    sc2::Point2D getStartLocation();
    float pathDistFromStartLocation(sc2::QueryInterface* query_, sc2::Point2D location_);
    bool spotReachable(const sc2::ObservationInterface* obs_, sc2::QueryInterface* query_, sc2::Point2D from_loc_, sc2::Point2D to_loc_);
    std::vector<Base> bases;

private:
    void initSetStartLocation();
    void initAddEnemyStartLocation(sc2::Point2D location_);
    void initMapChunks();
    sc2::Point2D getClosestUnseenLocation(bool pathable_=true);

    BasicSc2Bot* agent;
    std::vector<std::unique_ptr<MapChunk>> map_chunk_storage;
    std::unordered_set<MapChunk*> map_chunks;
    std::unordered_set<MapChunk*> pathable_map_chunks;
    std::unordered_map<int, MapChunk*> map_chunk_by_id;
    std::vector<sc2::Point2D> enemy_start_locations;
    sc2::Point2D enemy_start_location;
    sc2::Point2D proxy_location;
    sc2::Point2D start_location;
    int enemy_start_location_index;
    
};