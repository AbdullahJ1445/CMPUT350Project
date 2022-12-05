#pragma once
#include "sc2api/sc2_api.h"
#include "BasicSc2Bot.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"
#include "Base.h"

# define NO_POINT_FOUND sc2::Point2D(-2.5252, -2.5252) // value indicating no point found
# define CHUNK_SIZE 6.0f // the distance between adjacent chunks
# define THREAT_DECAY 0.50 // the amount threat decays for a chunk when in vision
# define NEARBY_THREAT_MODIFIER 0.05 // how much threat should increase for nearby chunks when enemies near

class BasicSc2Bot;

class MapChunk {
public:
    MapChunk(BasicSc2Bot* agent_, sc2::Point2D location_, bool pathable_);
    int seen_at();
    bool wasSeen();
    size_t getID();
    bool hasEnemyUnits();
    bool hasEnemyStructures();
    bool isPathable();
    double getThreat();
    float distSquaredFromStart();
    bool isNearStart();
    bool inVision(const sc2::ObservationInterface* obs);
    void checkVision(const sc2::ObservationInterface* obs);
    void increaseThreat(BasicSc2Bot* agent_, const sc2::Unit* unit, float modifier_);
    void setThreat(double amt_);
    sc2::Point2D getLocation();

private:
    float dist_squared_from_start;
    bool pathable;
    BasicSc2Bot* agent;
    sc2::Point2D location;
    int last_seen;
    size_t id;
    sc2::Visibility last_visibility;
    int enemy_units_last_seen_at;        // the last time step when enemy units were seen here
    int enemy_unit_count;
    int enemy_structure_count;
    double threat;                          // the calculated threat of this chunk
};

class LocationHandler {
public:
    LocationHandler(BasicSc2Bot* agent_);
    MapChunk* getNextUnseenChunk(bool pathable_=true);
    void setNextUnseenChunk(bool pathable_=true);
    sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
    int getIndexOfClosestBase(sc2::Point2D location_);
    void scanChunks(const sc2::ObservationInterface* obs);
    const sc2::Unit* getNearestMineralPatch(sc2::Point2D location);
    const sc2::Unit* getNearestGeyser(sc2::Point2D location);
    const sc2::Unit* getNearestGasStructure(sc2::Point2D location, bool allied=true);
    const sc2::Unit* getNearestTownhall(const sc2::Point2D location);
    sc2::Point2D getOldestLocation(bool pathable_=true);
    sc2::Point2D getHighestThreatLocation(bool pathable_=true, bool away_=true);

    sc2::Point2D smartAttackLocation(bool pathable_=true);
    sc2::Point2D smartAttackEnemyBase(bool pathable_=true);
    sc2::Point2D smartPriorityAttack(bool pathable_=true);
    sc2::Point2D smartAttackFlyingLocation();
    sc2::Point2D smartStayHomeAndDefend();

    void calculateHighestThreatForChunks();
    int getPlayerIDForMap(int map_index, sc2::Point2D location);
    void initLocations(int map_index, int p_id);
    void setEnemyStartLocation(sc2::Point2D location_);
    void setProxyLocation(sc2::Point2D);
    sc2::Point2D getEnemyStartLocationByIndex(int index_);
    sc2::Point2D getBestEnemyLocation();
    std::vector<MapChunk*> getLocalChunks(sc2::Point2D loc_);
    MapChunk* getNearestChunk(sc2::Point2D loc_);
    MapChunk* getNearestPathableChunk(sc2::Point2D loc_);
    MapChunk* getChunkByCoords(std::pair<float, float> coords);
    sc2::Point2D getAttackingForceLocation();
    sc2::Point2D getCenterOfArmy();
    sc2::Point2D getEnemyLocation();
    sc2::Point2D getProxyLocation();
    sc2::Point2D getStartLocation();
    sc2::Point2D getCenterPathableLocation();
    sc2::Point2D getRallyPointBeforeRallyPoint();
    bool locationsEqual(sc2::Point2D loc_1, sc2::Point2D loc_2);

    sc2::Point2D getRallyPointTowardsThreat();

    void setHighestThreat(double threat_);
    double getHighestThreat();
    void setHighestThreatChunk(MapChunk* chunk_);
    MapChunk* getHighestThreatChunk();

    void setHighestThreatAwayFromStart(double threat_);
    double getHighestThreatAwayFromStart();
    void setHighestThreatChunkAwayFromStart(MapChunk* chunk_);
    MapChunk* getHighestThreatChunkAwayFromStart();

    void setHighestPathableThreat(double threat_);
    double getHighestPathableThreat();
    void setHighestPathableThreatChunk(MapChunk* chunk_);
    MapChunk* getHighestPathableThreatChunk();

    void setHighestPathableThreatAwayFromStart(double threat_);
    double getHighestPathableThreatAwayFromStart();
    void setHighestPathableThreatChunkAwayFromStart(MapChunk* chunk_);
    MapChunk* getHighestPathableThreatChunkAwayFromStart();

    bool PathableThreatExistsNearLocation(sc2::Point2D loc_, float range_);
    MapChunk* getHighestPathableThreatChunkNearLocation(sc2::Point2D loc_, float range_);

    float distSquaredFromStart(sc2::Point2D loc_);
    sc2::Point2D getThreatNearStart();

    float pathDistFromStartLocation(sc2::QueryInterface* query_, sc2::Point2D location_);
    bool spotReachable(const sc2::ObservationInterface* obs_, sc2::QueryInterface* query_, sc2::Point2D from_loc_, sc2::Point2D to_loc_);
    std::vector<Base> bases;
    bool chunksInitialized();
    float getSqDistThreshold();

private:
    void initSetStartLocation();
    void initAddEnemyStartLocation(sc2::Point2D location_);
    void initMapChunks();
    sc2::Point2D getClosestUnseenLocation(bool pathable_=true);
    sc2::Point2D getFurthestUnseenLocation(bool pathable_=true);
    sc2::Point2D getClosestUnseenLocationToLastThreat(bool pathable_=true);

    BasicSc2Bot* agent;
    std::vector<std::unique_ptr<MapChunk>> map_chunk_storage;
    std::unordered_set<MapChunk*> map_chunks;
    std::unordered_set<MapChunk*> pathable_map_chunks;
    std::unordered_map<int, MapChunk*> map_chunk_by_id;
    std::map<std::pair<float, float>, MapChunk*> map_chunk_by_coords;
    std::vector<sc2::Point2D> enemy_start_locations;
    sc2::Point2D enemy_start_location;
    sc2::Point2D proxy_location;
    sc2::Point2D start_location;
    int enemy_start_location_index;
    bool chunks_initialized;
    int chunk_rows;
    int chunk_cols;
    float chunk_spread;
    float chunk_min_x;
    float chunk_min_y;
    float chunk_max_x;
    float chunk_max_y;
    double highest_threat;
    double highest_pathable_threat;
    double highest_threat_away_from_start;
    double highest_pathable_threat_away_from_start;
    MapChunk* high_threat_chunk;
    MapChunk* high_threat_pathable_chunk;
    MapChunk* high_threat_chunk_away_from_start;
    MapChunk* high_threat_pathable_chunk_away_from_start;
    float sq_dist_threshold;
    sc2::Point2D map_center;
    MapChunk* center_chunk;
    MapChunk* next_unseen_pathable_chunk;
    MapChunk* next_unseen_chunk;
    
};