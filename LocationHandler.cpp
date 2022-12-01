#pragma once
#include "sc2api/sc2_api.h"
#include "BasicSc2Bot.h"
#include "LocationHandler.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"



MapChunk::MapChunk(BasicSc2Bot* agent_, sc2::Point2D location_, bool pathable_) {
    static size_t id_ = 0;
    id = id_++;
    agent = agent_;
    location = location_;
    last_seen = -1;
    threat = 0;
    pathable = pathable_;
    dist_squared_from_start = agent->locH->distSquaredFromStart(location_);
}

sc2::Point2D MapChunk::getLocation() {
    return location;
}

int MapChunk::seen_at() {
    return last_seen;
}

size_t MapChunk::getID() {
    return id;
}

bool MapChunk::wasSeen() {
    return last_seen != -1;
}

bool MapChunk::isPathable() {
    return pathable;
}

bool MapChunk::hasEnemyUnits() {
    return enemy_unit_count > 0;
}

bool MapChunk::hasEnemyStructures() {
    return enemy_structure_count > 0;
}

double MapChunk::getThreat() {
    return threat;
}

float MapChunk::distSquaredFromStart() {
    return dist_squared_from_start;
}

bool MapChunk::isNearStart() {
    float threshold = agent->locH->getSqDistThreshold();
    return dist_squared_from_start < threshold;
}

void MapChunk::checkVision(const sc2::ObservationInterface* obs) {

    // checks the status of the chunk. If the chunk is visible, it updates the data for nearby enemy units.

    last_visibility = obs->GetVisibility(location);
    if (last_visibility == sc2::Visibility::Visible) {
        last_seen = obs->GetGameLoop();

        // decay threat when in vision
        if (threat > 0) {
            threat *= (1.0 - THREAT_DECAY);
            threat -= .1;
            if (threat < 0)
                threat = 0.0f;
        }
    }
}

bool MapChunk::inVision(const sc2::ObservationInterface* obs) {

    // sc2::Visibility enums
    // Hidden = 0,
    // Fogged = 1,
    // Visible = 2,
    // FullHidden = 3
    
    return (last_visibility == sc2::Visibility::Visible);
}

void MapChunk::increaseThreat(BasicSc2Bot* agent_, const sc2::Unit* unit, float modifier) {
    // increase the threat value of this chunk

    double d_threat = agent_->getValue(unit) * modifier;
    threat += d_threat;

    double highest_threat = agent->locH->getHighestThreat();
    double highest_pathable_threat = agent->locH->getHighestPathableThreat();
    double highest_away_threat = agent->locH->getHighestThreatAwayFromStart();
    double highest_pathable_away_threat = agent->locH->getHighestPathableThreatAwayFromStart();

    MapChunk* hi_chunk = agent->locH->getHighestThreatChunk();
    MapChunk* hi_p_chunk = agent->locH->getHighestPathableThreatChunk();
    MapChunk* hi_a_chunk = agent->locH->getHighestThreatChunkAwayFromStart();
    MapChunk* hi_p_a_chunk = agent->locH->getHighestPathableThreatChunkAwayFromStart();

    if (hi_chunk == this) {
        agent->locH->setHighestThreat(threat);
    }
    else {
        if (threat > highest_threat) {
            agent->locH->setHighestThreat(threat);
            agent->locH->setHighestThreatChunk(this);
        }
    }


    if (pathable) {
        if (hi_p_chunk == this) {
            agent->locH->setHighestPathableThreat(threat);
        }
        else {
            if (threat > highest_pathable_threat) {
                agent->locH->setHighestPathableThreat(threat);
                agent->locH->setHighestPathableThreatChunk(this);
            }
        }
    }
    if (!isNearStart()) {
        if (hi_a_chunk == this) {
            agent->locH->setHighestThreatAwayFromStart(threat);
        }
        else {
            if (threat > highest_away_threat) {
                agent->locH->setHighestThreatAwayFromStart(threat);
                agent->locH->setHighestThreatChunkAwayFromStart(this);
            }
        }
    }
    if (pathable && !isNearStart()) {
        if (hi_p_a_chunk == this) {
            agent->locH->setHighestPathableThreatAwayFromStart(threat);
        }
        else {
            if (threat > highest_pathable_away_threat) {
                agent->locH->setHighestPathableThreatAwayFromStart(threat);
                agent->locH->setHighestPathableThreatChunkAwayFromStart(this);
            }
        }
    }   
}

void MapChunk::setThreat(double amt_) {
    threat = amt_;
}


LocationHandler::LocationHandler(BasicSc2Bot* agent_){
    agent = agent_;
    enemy_start_location_index = 0;
    chunks_initialized = false;
    highest_threat = 0;
    highest_pathable_threat = 0;
    highest_threat_away_from_start = 0;
    highest_pathable_threat_away_from_start = 0;
    high_threat_chunk = nullptr;
    high_threat_pathable_chunk = nullptr;
    high_threat_chunk_away_from_start = nullptr;
    high_threat_pathable_chunk_away_from_start = nullptr;
    map_center = INVALID_POINT;
    center_chunk = nullptr;
}

sc2::Point2D LocationHandler::getNearestStartLocation(sc2::Point2D spot) {
    // Get the nearest Start Location from a given point
    float nearest_distance = 10000.0f;
    sc2::Point2D nearest_point;

    for (auto& iter : agent->Observation()->GetGameInfo().start_locations) {
        float dist = sc2::Distance2D(spot, iter);
        if (dist <= nearest_distance) {
            nearest_distance = dist;
            nearest_point = iter;
        }
    }
    return nearest_point;
}

int LocationHandler::getIndexOfClosestBase(sc2::Point2D location_) {
// get the index of the closest base to a location
    float distance = std::numeric_limits<float>::max();
    int lowest_index = 0;
    for (int i = 0; i < bases.size(); i++) {
        float b_dist = sc2::DistanceSquared2D(bases[i].getTownhall(), location_);
        if (b_dist < distance) {
            lowest_index = i;
            distance = b_dist;
        }
    }
    return lowest_index;
}

void LocationHandler::scanChunks(const sc2::ObservationInterface* obs) {
    //sc2::Units enemies = obs->GetUnits(sc2::Unit::Alliance::Enemy);
    for (auto it = map_chunks.begin(); it != map_chunks.end(); ++it) {
        (*it)->checkVision(obs);
    }
}

const sc2::Unit* LocationHandler::getNearestMineralPatch(sc2::Point2D location) {
    const sc2::ObservationInterface* obs = agent->Observation();
    sc2::Units units = agent->Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const sc2::Unit* target = nullptr;
    for (const auto& u : units) {
        if (agent->isMineralPatch(u)) {
            float d = sc2::DistanceSquared2D(u->pos, location);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}

const sc2::Unit* LocationHandler::getNearestGeyser(sc2::Point2D location) {
    const sc2::ObservationInterface* obs = agent->Observation();
    sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const sc2::Unit* target = nullptr;
    for (const auto& u : units) {
        if (agent->isGeyser(u)) {
            sc2::Point2D geyser_loc = u->pos;
            // check if geyser already is being mined
            const sc2::Unit* nearest_gas = getNearestGasStructure(geyser_loc);
            if (nearest_gas) {
                sc2::Point2D gas_structure_loc = nearest_gas->pos;
                // do not consider geyser with structure already built on it
                if (geyser_loc == gas_structure_loc) {
                    continue;
                }
            }
            float d = sc2::DistanceSquared2D(u->pos, location);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}    

const sc2::Unit* LocationHandler::getNearestGasStructure(sc2::Point2D location, bool allied) {
    const sc2::ObservationInterface* obs = agent->Observation();
    sc2::Units units;
    if (allied) {
        units = obs->GetUnits(sc2::Unit::Alliance::Self);
    }
    else {
        units = obs->GetUnits(sc2::Unit::Alliance::Self);
    }
    
    float distance = std::numeric_limits<float>::max();
    const sc2::Unit* target = nullptr;
    for (const auto& u : units) {
        sc2::UNIT_TYPEID unit_type = u->unit_type;
        if (unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
            unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY ||
            unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR ||
            unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH ||
            unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERYRICH ||
            unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTORRICH) {
            float d = sc2::DistanceSquared2D(u->pos, location);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}

const sc2::Unit* LocationHandler::getNearestTownhall(const sc2::Point2D location) {
    // find nearest townhall to location

    std::unordered_set<Mob*> townhalls = agent->mobH->filterByFlag(agent->mobH->getMobs(), FLAGS::IS_TOWNHALL);
    for (auto m : townhalls) {
        return &(m->unit);
    }
    return nullptr;
}

void LocationHandler::calculateHighestThreatForChunks() {
    // calculate the threat values for all combinations of conditions
    // pathable: the chunk is pathable
    // away: the chunk is away from the start location

    double max_threat = 0;
    double max_pathable_threat = 0;
    double max_threat_away = 0;
    double max_pathable_threat_away = 0;
    int max_id = -99;
    int max_pathable_id = -99;
    int max_id_away = -99;
    int max_pathable_id_away = -99;

    for (auto chunk : map_chunks) {
        int id = chunk->getID();
        float chunk_threat = chunk->getThreat();
        bool pathable = chunk->isPathable();
        bool away = !chunk->isNearStart();
        if (chunk_threat > max_threat) {
            max_threat = chunk_threat;
            max_id = id;
        }
        if (pathable && chunk_threat > max_pathable_threat) {
            max_pathable_threat = chunk_threat;
            max_pathable_id = id;
        }
        if (away && chunk_threat > max_threat_away) {
            max_threat_away = chunk_threat;
            max_id_away = id;
        }
        if (pathable && away && chunk_threat > max_pathable_threat_away) {
            max_pathable_threat_away = chunk_threat;
            max_pathable_id_away = id;
        }
    }

    if (max_id != -99) {
        high_threat_chunk = map_chunk_by_id[max_id];
        highest_threat = max_threat;
    }
    if (max_pathable_id != -99) {
        high_threat_pathable_chunk = map_chunk_by_id[max_pathable_id];
        highest_pathable_threat = max_pathable_threat;
    }
    if (max_id_away != -99) {
        high_threat_chunk_away_from_start = map_chunk_by_id[max_id_away];
        highest_threat_away_from_start = max_threat_away;
    }
    if (max_pathable_id_away != -99) {
        high_threat_pathable_chunk_away_from_start = map_chunk_by_id[max_pathable_id_away];
        highest_pathable_threat_away_from_start = max_pathable_threat_away;
    }

}

sc2::Point2D LocationHandler::getHighestThreatLocation(bool pathable_, bool away_) {
    
    sc2::Point2D return_loc = NO_POINT_FOUND;
    float hi_threat = 0;
    MapChunk* hi_chunk = nullptr;
    if (pathable_) {
        if (away_) {
            hi_chunk = getHighestPathableThreatChunkAwayFromStart();
            hi_threat = hi_chunk->getThreat();
        }
        else {
            hi_chunk = getHighestPathableThreatChunk();
            hi_threat = hi_chunk->getThreat();
        }
    }
    else {
        if (away_) {
            hi_chunk = getHighestThreatChunkAwayFromStart();
            hi_threat = hi_chunk->getThreat();
        }
        else {
            hi_chunk = getHighestThreatChunk();
            hi_threat = hi_chunk->getThreat();
        }
    }

    if (hi_threat > 0) {
        return_loc = hi_chunk->getLocation();
    }

    return return_loc;
}

MapChunk* LocationHandler::getChunkByCoords(std::pair<float, float> coords) {
    return map_chunk_by_coords[coords];
}

sc2::Point2D LocationHandler::getCenterOfArmy() {
    // returns the "center of mass" of army units

    sc2::Point2D point_sum(0.0f, 0.0f);

    std::unordered_set<Mob*> attackers = agent->mobH->filterByFlag(agent->mobH->getMobs(), FLAGS::IS_ATTACKER);
    for (auto it = attackers.begin(); it != attackers.end(); ++it) {
        point_sum += (*it)->unit.pos;
    }
    int army_size = attackers.size();
    return point_sum / army_size;
}

sc2::Point2D LocationHandler::getEnemyLocation()
{
    if (!enemy_start_locations.empty()) {
        return enemy_start_locations.front();
    }
    else {
        return NO_POINT_FOUND;
    }
}

sc2::Point2D LocationHandler::smartAttackLocation(bool pathable_) {
    // first returns the highest threat location
    // if none found, then returns the oldest location, meaning the location
    // which was visited the least recently

    sc2::Point2D high_threat = getHighestThreatLocation(pathable_);
    if (high_threat != NO_POINT_FOUND)
        return high_threat;
    return getOldestLocation(pathable_);
}

sc2::Point2D LocationHandler::smartAttackEnemyBase(bool pathable_) {
    // first returns the highest threat location away from start
    // if none found, then returns the highest threat location
    // if none found, then returns the oldest location

    sc2::Point2D high_threat = getHighestThreatLocation(pathable_, true);
    if (high_threat != NO_POINT_FOUND)
        return high_threat;
    high_threat = getHighestThreatLocation(pathable_);
    if (high_threat != NO_POINT_FOUND)
        return high_threat;
    return getOldestLocation(pathable_);
}

sc2::Point2D LocationHandler::smartPriorityAttack(bool pathable_) {
    // will choose whether to attack the enemy base or defend
    // the player's base, depending on the distance from the center
    // of the army's mass
    // helps keep army committed to attack instead of spreading itself thin

    sc2::Point2D threat_away = getHighestThreatLocation(pathable_, true);
    sc2::Point2D threat_home = getThreatNearStart();
    sc2::Point2D threat_any = getHighestThreatLocation(pathable_);
    if (threat_away == NO_POINT_FOUND) {
        if (threat_home == NO_POINT_FOUND) {
            if (threat_any == NO_POINT_FOUND) {
                return getOldestLocation(pathable_);
            }
            else {
                return threat_any;
            }
        }
        else {
            return threat_home;
        }
    }
    else {
        if (threat_home == NO_POINT_FOUND) {
            return threat_away;
        }
        else {
            sc2::Point2D center_of_mass = getCenterOfArmy();
            if (sc2::DistanceSquared2D(threat_home, center_of_mass) < sc2::DistanceSquared2D(threat_away, center_of_mass)) {
                return threat_home;
            }
            else {
                return threat_away;
            }
        }
    }
}

sc2::Point2D LocationHandler::smartAttackFlyingLocation() {
    // simply calls smartAttackLocation() with pathable_ = false
    // allows for easier use in strategies because no parameters needed
    return smartAttackLocation(false);
}

sc2::Point2D LocationHandler::smartStayHomeAndDefend()
{
    sc2::Point2D hi_threat = getThreatNearStart();
    if (hi_threat == NO_POINT_FOUND) {
        return ASSIGNED_LOCATION;
    }
    return hi_threat;
}

sc2::Point2D LocationHandler::getOldestLocation(bool pathable_)
{
    // first check if any locations were never seen
    bool unseen = false;

    //std::cout << "getting oldest location ";

    std::unordered_set<MapChunk*> chunkset;
    if (pathable_)
        chunkset = pathable_map_chunks;
    else
        chunkset = map_chunks;

    //std::cout << "<" << chunkset.size() << ">";
    for (auto it = chunkset.begin(); it != chunkset.end(); ++it) {
        if (!(*it)->wasSeen()) {
            unseen = true;
            break;
        }
    }

    if (unseen) {
        
        //return getClosestUnseenLocation(pathable_);
        return getFurthestUnseenLocation(pathable_);
        //return getClosestUnseenLocationToLastThreat(pathable_);
    }
    //std::cout << "(no-unseen)";

    int lowest = std::numeric_limits<int>::max();
    sc2::Point2D lowest_loc = NO_POINT_FOUND;

    for (auto it = chunkset.begin(); it != chunkset.end(); ++it) {
        if ((*it)->seen_at() < lowest) {
            lowest_loc = (*it)->getLocation();
            lowest = (*it)->seen_at();
        }
    }

    //std::cout << "(return[" << lowest_loc.x << "," << lowest_loc.y << "])";

    return lowest_loc;
}

sc2::Point2D LocationHandler::getClosestUnseenLocation(bool pathable_) {
    // gets the closest chunk location to any of the player's units
    std::unordered_set<MapChunk*> chunkset;
    std::unordered_set<MapChunk*> unseen_chunks;
    if (pathable_)
        chunkset = pathable_map_chunks;
    else
        chunkset = map_chunks;

    for (auto it = chunkset.begin(); it != chunkset.end(); ++it) {
        if (!(*it)->wasSeen()) {
            unseen_chunks.insert(*it);
        }
    }

    if (unseen_chunks.empty()) {
        std::cout << "unseen_chunks.empty() ";
        return NO_POINT_FOUND;
    }

    std::unordered_set<Mob*> mobs = agent->mobH->getMobs();
    std::unordered_set<Mob*> flying_mobs = agent->mobH->filterByFlag(mobs, FLAGS::IS_FLYING);
    
    // should not happen, but lets make sure... this would mean game over
    if (mobs.empty())
        return NO_POINT_FOUND;

    float closest_dist = std::numeric_limits<float>::max();
    sc2::Point2D closest_loc = NO_POINT_FOUND;

    for (auto it = unseen_chunks.begin(); it != unseen_chunks.end(); ++it) {
        sc2::Point2D loc = (*it)->getLocation();
        Mob* closest_mob = nullptr;
        if (!pathable_ && !(*it)->isPathable()) {
            // if a chunk can only be reached by flying units
            closest_mob = Directive::getClosestToLocation(flying_mobs , loc);
        }
        else {
            closest_mob = Directive::getClosestToLocation(mobs, loc);
        }

        if (closest_mob == nullptr)
            continue;

        float dist = sc2::DistanceSquared2D(closest_mob->unit.pos, loc);
        if (dist < closest_dist) {
            closest_loc = loc;
            closest_dist = dist;
        }
    }

    return closest_loc;
}

sc2::Point2D LocationHandler::getFurthestUnseenLocation(bool pathable_) {
    // checks for the unseen MapChunk point furthest from the start location

    std::unordered_set<MapChunk*> chunkset;
    std::unordered_set<MapChunk*> unseen_chunks;
    if (pathable_)
        chunkset = pathable_map_chunks;
    else
        chunkset = map_chunks;

    for (auto it = chunkset.begin(); it != chunkset.end(); ++it) {
        if (!(*it)->wasSeen()) {
            unseen_chunks.insert(*it);
        }
    }

    if (unseen_chunks.empty()) {
        std::cout << "unseen_chunks.empty() ";
        return NO_POINT_FOUND;
    }

    std::unordered_set<Mob*> mobs = agent->mobH->getMobs();
    std::unordered_set<Mob*> flying_mobs = agent->mobH->filterByFlag(mobs, FLAGS::IS_FLYING);

    // should not happen, but lets make sure... this would mean game over
    if (mobs.empty())
        return NO_POINT_FOUND;

    float furthest_dist = 0;
    sc2::Point2D furthest_loc = NO_POINT_FOUND;

    for (auto it = unseen_chunks.begin(); it != unseen_chunks.end(); ++it) {
        sc2::Point2D loc = (*it)->getLocation();
        float dist = sc2::DistanceSquared2D(start_location, loc);
        if (dist > furthest_dist) {
            furthest_loc = loc;
            furthest_dist = dist;
        }
    }

    return furthest_loc;
}

sc2::Point2D LocationHandler::getClosestUnseenLocationToLastThreat(bool pathable_) {
    // gets the closest chunk location to any of the player's units
    std::unordered_set<MapChunk*> chunkset;
    std::unordered_set<MapChunk*> unseen_chunks;
    if (pathable_)
        chunkset = pathable_map_chunks;
    else
        chunkset = map_chunks;

    for (auto it = chunkset.begin(); it != chunkset.end(); ++it) {
        if (!(*it)->wasSeen()) {
            unseen_chunks.insert(*it);
        }
    }

    if (unseen_chunks.empty()) {
        std::cout << "unseen_chunks.empty() ";
        return NO_POINT_FOUND;
    }

    float closest_dist = std::numeric_limits<float>::max();
    sc2::Point2D closest_loc = NO_POINT_FOUND;

    sc2::Point2D high_threat_loc = agent->locH->getHighestThreatChunk()->getLocation();

    for (auto it = unseen_chunks.begin(); it != unseen_chunks.end(); ++it) {
        sc2::Point2D loc = (*it)->getLocation();

        float dist = sc2::DistanceSquared2D(high_threat_loc, loc);
        if (dist < closest_dist) {
            closest_loc = loc;
            closest_dist = dist;
        }
    }

    return closest_loc;
}

bool LocationHandler::locationsEqual(sc2::Point2D loc_1, sc2::Point2D loc_2) {
    // ensure locations are equaivalent, despite potential floating point serrors

    int loc_1x_10 = loc_1.x * 10;
    int loc_1y_10 = loc_1.y * 10;
    int loc_2x_10 = loc_2.x * 10;
    int loc_2y_10 = loc_2.y * 10;
    return (loc_1x_10 == loc_2x_10 && loc_1y_10 == loc_2y_10);
}


int LocationHandler::getPlayerIDForMap(int map_index, sc2::Point2D location) {
    //location = getNearestStartLocation(location);
    //location = agent->Observation()->GetStartLocation();
        
    int p_id = 0;
    switch (map_index) {
    case 1:
        // Cactus Valley LE
        if (locationsEqual(location, sc2::Point2D(33.5, 158.5))) {
            // top left
            p_id = 1;
        }
        if (locationsEqual(location, sc2::Point2D(158.5, 158.5))) {
            // top right
            p_id = 2;
        }
        if (locationsEqual(location, sc2::Point2D(158.5, 33.5))) {
            // bottom right
            p_id = 3;
        }
        if (locationsEqual(location, sc2::Point2D(33.5, 33.5))) {
            // bottom left
            p_id = 4;
        }
        break;
    case 2:
        // Bel'Shir Vestige LE
        if (locationsEqual(location, sc2::Point2D(114.5, 25.5))) {
            // bottom right
            p_id = 1;
        }
        if (locationsEqual(location, sc2::Point2D(29.5, 134.5))) {
            // top left
            p_id = 2;
        }
        break;
    case 3:
        // Proxima Station LE
        if (locationsEqual(location, sc2::Point2D(137.5, 139.5))) {
            // top right
            p_id = 1;
        }
        if (locationsEqual(location, sc2::Point2D(62.5, 28.5))) {
            // bottom left
            p_id = 2;
        }
        break;
    }
    assert(p_id != 0);
    return p_id;
}

void LocationHandler::initLocations(int map_index, int p_id) {
    const sc2::ObservationInterface* observation = agent->Observation();
    initSetStartLocation();
    
    std::cout << "map index: " << map_index << std::endl;
    std::cout << "p_id: " << p_id << std::endl;

    if (map_index == 1) {
        // CACTUS VALLEY
        if (p_id == 1) {
            
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(158.5, 158.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(158.5, 33.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(33.5, 33.5));

            Base main_base(observation->GetStartLocation());
            main_base.addBuildArea(32, 147);
            main_base.addBuildArea(47, 151);
            main_base.addBuildArea(44.5, 160.5);
            main_base.setRallyPoint(73, 158);
            bases.push_back(main_base);

            Base exp_1(66.5, 161.5);
            exp_1.addDefendPoint(64, 152);
            exp_1.setRallyPoint(73, 158);
            bases.push_back(exp_1); // radius 6.0F

            /*
            Base exp_2(54.5, 132.5);
            bases.push_back(exp_2);

            Base exp_3(45.5, 20.5);
            bases.push_back(exp_3);

            Base exp_4(93.5, 156.5);
            bases.push_back(exp_4);

            Base exp_5(35.5, 93.5);
            bases.push_back(exp_5);

            Base exp_6(30.5, 66.5);
            bases.push_back(exp_6);

            Base exp_7(132.5, 137.5);
            bases.push_back(exp_7);

            Base exp_8(59.5, 54.5);
            bases.push_back(exp_8);

            Base exp_9(161.5, 125.5);
            bases.push_back(exp_9);

            Base exp_10(156.5, 98.5);
            bases.push_back(exp_10);

            Base exp_11(98.5, 35.5);
            bases.push_back(exp_11);

            Base exp_12(125.5, 30.5);
            bases.push_back(exp_12);

            Base exp_13(137.5, 59.5);
            bases.push_back(exp_13);
            */
        }
        else if (p_id == 2) {
            
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(158.5, 33.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(33.5, 33.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(33.5, 158.5));

            Base main_base(observation->GetStartLocation());
            main_base.addBuildArea(147, 160);
            main_base.addBuildArea(151, 145);
            main_base.addBuildArea(160.5, 147.5);
            main_base.setRallyPoint(158, 119);
            bases.push_back(main_base);

            Base exp_1(161.5, 125.5);
            exp_1.addDefendPoint(152, 128); // radius 6.0F
            exp_1.setRallyPoint(158, 119);
            bases.push_back(exp_1);
        }
        else if (p_id == 3) {
            
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(33.5, 33.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(33.5, 158.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(158.5, 158.5));

            Base main_base(observation->GetStartLocation());
            main_base.addBuildArea(160, 45);
            main_base.addBuildArea(145, 41);
            main_base.setRallyPoint(119, 34);
            bases.push_back(main_base);

            Base exp_1(125.5, 30.5);
            exp_1.addDefendPoint(128, 40); // radius 6.0F
            exp_1.setRallyPoint(119, 34);
            bases.push_back(exp_1);
        }
        else if (p_id == 4) {
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(33.5, 158.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(158.5, 158.5));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(158.5, 33.5));
            

            Base main_base(observation->GetStartLocation());
            main_base.addBuildArea(45, 32);
            main_base.addBuildArea(41, 47);
            main_base.addBuildArea(31.5, 44.5);
            main_base.setRallyPoint(34, 73);
            bases.push_back(main_base);

            Base exp_1(30.5, 66.5);
            exp_1.addDefendPoint(40, 64);  // radius 6.0F
            exp_1.setRallyPoint(34, 73);
            bases.push_back(exp_1);
        }
    }

    else if (map_index == 2) {
        // BELSHIR VESTIGE

        if (p_id == 1) { // bottom
            setProxyLocation(sc2::Point2D(33.0, 98.0));
            initAddEnemyStartLocation(enemy_start_location = sc2::Point2D(29.5, 134.5));
            Base main_base(observation->GetStartLocation());
            bases.push_back(main_base);

            Base exp_1(82.5, 23.5);
            bases.push_back(exp_1);

            Base exp_2(115.5, 63.5);
            bases.push_back(exp_2);

            Base exp_3(45.5, 20.5);
            bases.push_back(exp_3);

            Base exp_4(120.5, 104.5);
            bases.push_back(exp_4);

            Base exp_5(98.5, 138.5);
            bases.push_back(exp_5);

            Base exp_6(23.5, 55.5);
            bases.push_back(exp_6);

            Base exp_7(28.5, 96.5);
            exp_7.setRallyPoint(40.0F, 102.0F);
            bases.push_back(exp_7);

            Base exp_8(61.5, 136.5);
            bases.push_back(exp_8);

        } 
        else if (p_id == 2) { // top

            setProxyLocation(sc2::Point2D(111.0, 62.0));
            initAddEnemyStartLocation(sc2::Point2D(114.5, 25.5));
            Base main_base(observation->GetStartLocation());
            bases.push_back(main_base);

            Base exp_1(61.5, 136.5);
            bases.push_back(exp_1);

            Base exp_2(28.5, 96.5);
            bases.push_back(exp_2);

            Base exp_3(23.5, 55.5);
            bases.push_back(exp_3);

            Base exp_4(98.5, 138.5);
            bases.push_back(exp_4);

            Base exp_5(120.5, 104.5);
            bases.push_back(exp_5);

            Base exp_6(45.5, 20.5);
            bases.push_back(exp_6);

            Base exp_7(115.5, 63.5);
            exp_7.setRallyPoint(104.0F, 58.0F);
            bases.push_back(exp_7);

            Base exp_8(82.5, 23.5);
            bases.push_back(exp_8);
        }
    }
    else if (map_index == 3) {
        
        // PROXIMA STATION

        if (p_id == 1) { // top
            setProxyLocation(sc2::Point2D(45.0, 103.0));
            initAddEnemyStartLocation(sc2::Point2D(62.5, 28.5));

            Base main_base(observation->GetStartLocation());
            main_base.addBuildArea(146.0, 128.0);
            main_base.addBuildArea(132.0, 132.0);
            main_base.addDefendPoint(149.0, 120.0);
            main_base.setRallyPoint(149.0, 120.0);
            main_base.setActive();
            bases.push_back(main_base);

            Base exp_1(164.5, 140.5);
            exp_1.addBuildArea(165.0, 135.0);
            exp_1.addDefendPoint(168.0, 132.0);
            bases.push_back(exp_1);

            Base exp_2(149.5, 102.5);
            exp_2.addBuildArea(144.0, 106.0);
            exp_2.addDefendPoint(141.0, 95.0);
            bases.push_back(exp_2);

            Base exp_3(166.5, 69.5);
            exp_3.addBuildArea(161.0, 79.0);
            exp_3.addDefendPoint(155.0, 69.0);
            bases.push_back(exp_3);

            Base exp_4(119.5, 111.5);
            exp_4.addDefendPoint(115.0, 105.0);
            exp_4.addDefendPoint(110.0, 115.0);
            bases.push_back(exp_4);

            Base exp_5(127.5, 57.5);
            exp_5.addDefendPoint(122.0, 69.0);
            exp_5.addBuildArea(137.0, 54.0);
            bases.push_back(exp_5);

            Base exp_6(165.5, 23.5);
            exp_6.addDefendPoint(157.0, 23.0);
            bases.push_back(exp_6);

            Base exp_7(93.5, 147.5);
            exp_7.addDefendPoint(94.0, 141.0);
            bases.push_back(exp_7);

            Base exp_8(106.5, 20.5);
            exp_8.addDefendPoint(106.0, 27.0);
            bases.push_back(exp_8);

            Base exp_9(34.5, 144.5);
            exp_9.addDefendPoint(43.0, 145.0);
            bases.push_back(exp_9);

            Base exp_10(72.5, 110.5);
            exp_10.addDefendPoint(78.0, 99.0);
            exp_10.addDefendPoint(63.0, 114.0);
            bases.push_back(exp_10);

            Base exp_11(80.5, 56.5);
            exp_11.addDefendPoint(85.0, 63.0);
            exp_11.addDefendPoint(90.0, 53.0);
            bases.push_back(exp_11);

            Base exp_12(33.5, 98.5);
            exp_12.setRallyPoint(49.0, 91.0);
            exp_12.addBuildArea(39.0, 89.0);
            exp_12.addDefendPoint(45.0, 99.0);
            bases.push_back(exp_12);

            Base exp_13(50.5, 65.5);
            exp_13.addBuildArea(56.0, 62.0);
            exp_13.addDefendPoint(59.0, 73.0);
            exp_13.setRallyPoint(27.5, 66.5); // our proxy is close to this base, so want to set rally here
            bases.push_back(exp_13);

            Base exp_14(35.5, 27.5);
            exp_14.addBuildArea(35.0, 33.0);
            exp_14.addDefendPoint(32.0, 36.0);
            exp_14.setRallyPoint(27.5, 66.5); // our proxy is close to this base, so want to set rally here
            bases.push_back(exp_14);
        }
        else if (p_id == 2) { // bottom
            setProxyLocation(sc2::Point2D(155.0, 65.0));
            initAddEnemyStartLocation(sc2::Point2D(137.5, 139.5));
            Base main_base(observation->GetStartLocation());
            main_base.addBuildArea(54.0, 40.0);
            main_base.addBuildArea(68.0, 36.0);
            main_base.addDefendPoint(51.0, 48.0);
            main_base.setRallyPoint(51.0, 48.0);
            main_base.setActive();
            bases.push_back(main_base);

            Base exp_1(35.5, 27.5);
            exp_1.addBuildArea(35.0, 33.0);
            exp_1.addDefendPoint(32.0, 36.0);
            bases.push_back(exp_1);

            Base exp_2(50.5, 65.5);
            exp_2.addBuildArea(56.0, 62.0);
            exp_2.addDefendPoint(59.0, 73.0);
            bases.push_back(exp_2);

            Base exp_3(33.5, 98.5);
            exp_3.addBuildArea(39.0, 89.0);
            exp_3.addDefendPoint(45.0, 99.0);
            bases.push_back(exp_3);

            Base exp_4(80.5, 56.5);
            exp_4.addDefendPoint(85.0, 63.0);
            exp_4.addDefendPoint(90.0, 53.0);
            bases.push_back(exp_4);

            Base exp_5(72.5, 110.5);
            exp_5.addDefendPoint(78.0, 99.0);
            exp_5.addDefendPoint(63.0, 114.0);
            bases.push_back(exp_5);

            Base exp_6(34.5, 144.5);
            exp_6.addDefendPoint(43.0, 145.0);
            bases.push_back(exp_6);

            Base exp_7(106.5, 20.5);
            exp_7.addDefendPoint(106.0, 27.0);
            bases.push_back(exp_7);

            Base exp_8(93.5, 147.5);
            exp_8.addDefendPoint(94.0, 141.0);
            bases.push_back(exp_8);

            Base exp_9(165.5, 23.5);
            exp_9.addDefendPoint(157.0, 23.0);
            bases.push_back(exp_9);

            Base exp_10(127.5, 57.5);
            exp_10.addDefendPoint(122.0, 69.0);
            exp_10.addBuildArea(137.0, 54.0);
            bases.push_back(exp_10);

            Base exp_11(119.5, 111.5);
            exp_11.addDefendPoint(115.0, 105.0);
            exp_11.addDefendPoint(110.0, 115.0);
            bases.push_back(exp_11);

            Base exp_12(166.5, 69.5);
            exp_12.setRallyPoint(151.0, 77.0);
            exp_12.addBuildArea(161.0, 79.0);
            exp_12.addDefendPoint(155.0, 69.0);
            bases.push_back(exp_12);

            Base exp_13(149.5, 102.5);
            exp_13.addBuildArea(144.0, 106.0);
            exp_13.addDefendPoint(141.0, 95.0);
            exp_13.setRallyPoint(172.5, 101.5); // our proxy is close to this base, so want to set rally here
            bases.push_back(exp_13);

            Base exp_14(164.5, 140.5);
            exp_14.addBuildArea(165.0, 135.0);
            exp_14.addDefendPoint(168.0, 132.0);
            exp_14.setRallyPoint(172.5, 101.5); // our proxy is close to this base, so want to set rally here
            bases.push_back(exp_14);
        }
    }
    
    float closest_sq_dist = std::numeric_limits<float>::max();
    for (auto esl : enemy_start_locations) {
        if (distSquaredFromStart(esl) < closest_sq_dist) {
            closest_sq_dist = distSquaredFromStart(esl);
        }
    }

    sq_dist_threshold = (closest_sq_dist * .4);

    initMapChunks();
    scanChunks(agent->Observation());
}

void LocationHandler::initAddEnemyStartLocation(sc2::Point2D location_) {
    enemy_start_locations.push_back(location_);
}

float LocationHandler::pathDistFromStartLocation(sc2::QueryInterface* query_, sc2::Point2D location_) {
    return query_->PathingDistance(start_location, location_);
}

float LocationHandler::getSqDistThreshold() {
    return sq_dist_threshold;
}

bool LocationHandler::spotReachable(const sc2::ObservationInterface* obs_, sc2::QueryInterface* query_, sc2::Point2D from_loc_, sc2::Point2D to_loc_) {
    bool reachable = true;
    if (!obs_->IsPathable(to_loc_))
        return false;
    if (query_->PathingDistance(from_loc_, to_loc_) == 0)
        return false;
    return true;
}

void LocationHandler::initMapChunks()
{
    std::cout << "initializing map chunks";
    int pathable_count = 0;

    const sc2::ObservationInterface* obs = agent->Observation();

    sc2::GameInfo game_info = obs->GetGameInfo();
    float chunk_size = CHUNK_SIZE;
    float min_x = game_info.playable_min.x + chunk_size;
    float min_y = game_info.playable_min.x + chunk_size;
    float max_x;
    float max_y;
    map_center = sc2::Point2D((game_info.playable_max.x - game_info.playable_min.x) / 2 + game_info.playable_min.x, (game_info.playable_max.y - game_info.playable_min.y) / 2 + game_info.playable_min.y);

    // chunk matrix size
    int width = 0;
    int height = 0;   

    // set maximums to correspond to chunk locations
    for (float x_ = min_x; x_ < game_info.playable_max.x; x_ += chunk_size) { 
        max_x = x_;
        width++;
    }
    for (float y_ = min_y; y_ < game_info.playable_max.y; y_ += chunk_size) { 
        max_y = y_; 
        height++;
    }

    // store information
    chunk_spread = chunk_size;
    chunk_min_x = min_x;
    chunk_min_y = min_y;
    chunk_max_x = max_x;
    chunk_max_y = max_y;
    chunk_cols = width;
    chunk_rows = height;

    sc2::QueryInterface* query = agent->Query();


    
    // offsets to ensure a spot is fully reachable
    std::vector<sc2::Point2D> offsets;

    offsets.push_back(sc2::Point2D(0, 0));
    for (float x_off = -1.0; x_off <= 1.0; x_off += 2.0) {
        for (float y_off = -1.0; y_off <= 1.0; y_off += 2.0) {
            offsets.push_back(sc2::Point2D(x_off, y_off));
        }
    }
    
    assert(offsets.size() == 5);

    for (int j = 0; j < height; j++) {
        if (j % 2 == 0)
            std::cout << ".";
        for (int i = 0; i < width; i++) {
            sc2::Point2D loc_ = sc2::Point2D(min_x + (i * chunk_size), min_y + (j * chunk_size));
            bool pathable_ = true;

           
            for (auto off_ : offsets) {
                if (!spotReachable(obs, query, start_location, loc_ + off_)) {
                    pathable_ = false;
                    break;
                }
            }

            if (pathable_)
                pathable_count++;
                
            MapChunk chunk(agent, loc_, pathable_);
            map_chunk_storage.emplace_back(std::make_unique<MapChunk>(chunk));
            MapChunk* chunk_ptr = map_chunk_storage.back().get();
            map_chunks.insert(chunk_ptr);
            if (pathable_) {
                pathable_map_chunks.insert(chunk_ptr);
            }
            map_chunk_by_id[chunk_ptr->getID()] = chunk_ptr;
            sc2::Point2D chunk_loc = chunk_ptr->getLocation();
            map_chunk_by_coords[std::pair<float, float>(chunk_loc.x, chunk_loc.y)] = chunk_ptr;
        }
    }


    if (!enemy_start_locations.empty()) {
        double threat_amt = enemy_start_locations.size() * 2000 + 2000;
        for (auto esl : enemy_start_locations) {
            MapChunk* chunk = getNearestPathableChunk(esl);
            chunk->setThreat(threat_amt);
            threat_amt -= 2000;
        }
    }

    chunks_initialized = true;
    std::cout << std::endl << map_chunks.size() << " chunks initialized (" << pathable_count << " pathable)" << std::endl;
    agent->setLoadingProgress(4);
}

void LocationHandler::setEnemyStartLocation(sc2::Point2D location_)
{
    sc2::Point2D enemy_loc = getNearestStartLocation(location_);

    for (int i = 0; i < enemy_start_locations.size(); i++) {
        if (enemy_start_locations[i] == enemy_loc) {
            enemy_start_location_index = i;
        }
    }
}

std::vector<MapChunk*> LocationHandler::getLocalChunks(sc2::Point2D loc_) {
    // get 4 nearby chunks bounding a point, in a vector, the closest at index 0
    std::vector<MapChunk*> chunks;

    float closest_mod = 1.0f;
    float others_mod = 0.5f;

    // adjusted point relative to the chunk grid
    sc2::Point2D adjusted_loc = loc_ - sc2::Point2D(chunk_min_x, chunk_min_y);

    // force location to a point inside our chunk grid
    if (adjusted_loc.x < 0)
        adjusted_loc.x = 0.1;
    if (adjusted_loc.y < 0)
        adjusted_loc.y = 0.1;
    if (adjusted_loc.x > chunk_max_x - chunk_min_x)
        adjusted_loc.x = chunk_max_x - chunk_min_x - .1;
    if (adjusted_loc.y > chunk_max_y - chunk_min_y)
        adjusted_loc.y = chunk_max_y - chunk_min_y - .1;

    float right_bound;
    float left_bound;
    float up_bound;
    float down_bound;

    for (float x = 0.0f; x < adjusted_loc.x; x += chunk_spread) {
        left_bound = x;
    }
    right_bound = left_bound + chunk_spread;

    for (float y = 0.0f; y < adjusted_loc.y; y += chunk_spread) {
        up_bound = y;
    }
    down_bound = up_bound + chunk_spread;


    // check which halves of the square between chunks the point is in
    bool left_half = (adjusted_loc.x - left_bound < chunk_spread / 2);
    bool top_half = (adjusted_loc.y - up_bound < chunk_spread / 2);
    sc2::Point2D chunk_loc;
    sc2::Point2D other_1;
    sc2::Point2D other_2;
    sc2::Point2D other_3;

    // determine the coordinates of the other bounding chunks
    if (left_half) {
        chunk_loc.x = left_bound + chunk_min_x;
        other_1.x = left_bound + chunk_min_x;
        other_2.x = right_bound + chunk_min_x;
        other_3.x = right_bound + chunk_min_x;
    }
    else {
        chunk_loc.x = right_bound + chunk_min_x;
        other_1.x = right_bound + chunk_min_x;
        other_2.x = left_bound + chunk_min_x;
        other_3.x = left_bound + chunk_min_x;
    }
    if (top_half) {
        chunk_loc.y = up_bound + chunk_min_y;
        other_1.y = down_bound + chunk_min_y;
        other_2.y = up_bound + chunk_min_y;
        other_3.y = down_bound + chunk_min_y;
    }
    else {
        chunk_loc.y = down_bound + chunk_min_y;
        other_1.y = up_bound + chunk_min_y;
        other_2.y = down_bound + chunk_min_y;
        other_3.y = up_bound + chunk_min_y;
    }

    // 4 resulting chunk
    MapChunk* closest = map_chunk_by_coords[std::pair<float, float>(chunk_loc.x, chunk_loc.y)];
    MapChunk* near_1 = map_chunk_by_coords[std::pair<float, float>(other_1.x, other_1.y)];
    MapChunk* near_2 = map_chunk_by_coords[std::pair<float, float>(other_2.x, other_2.y)];
    MapChunk* near_3 = map_chunk_by_coords[std::pair<float, float>(other_3.x, other_3.y)];

    chunks.push_back(closest);
    chunks.push_back(near_1);
    chunks.push_back(near_2);
    chunks.push_back(near_3);

    return chunks;
}

MapChunk* LocationHandler::getNearestChunk(sc2::Point2D loc_) {

    // adjusted point relative to the chunk grid
    sc2::Point2D adjusted_loc = loc_ - sc2::Point2D(chunk_min_x, chunk_min_y);

    // force location to a point inside our chunk grid
    if (adjusted_loc.x < 0)
        adjusted_loc.x = 0.1;
    if (adjusted_loc.y < 0)
        adjusted_loc.y = 0.1;
    if (adjusted_loc.x > chunk_max_x - chunk_min_x)
        adjusted_loc.x = chunk_max_x - chunk_min_x - .1;
    if (adjusted_loc.y > chunk_max_y - chunk_min_y)
        adjusted_loc.y = chunk_max_y - chunk_min_y - .1;

    float right_bound;
    float left_bound;
    float up_bound;
    float down_bound;

    for (float x = 0.0f; x < adjusted_loc.x; x+= chunk_spread) {
        left_bound = x;
    }
    right_bound = left_bound + chunk_spread;

    for (float y = 0.0f; y < adjusted_loc.y; y += chunk_spread) {
        up_bound = y;
    }
    down_bound = up_bound + chunk_spread;

    
    // check which halves of the square between chunks the point is in
    bool left_half = (adjusted_loc.x - left_bound < chunk_spread / 2);
    bool top_half = (adjusted_loc.y - up_bound < chunk_spread / 2);
    sc2::Point2D chunk_loc;
    
    if (left_half) {
        chunk_loc.x = left_bound + chunk_min_x;
    }
    else {
        chunk_loc.x = right_bound + chunk_min_x;
    }
    if (top_half) {
        chunk_loc.y = up_bound + chunk_min_y;
    }
    else {
        chunk_loc.y = down_bound + chunk_min_y;
    }

    MapChunk* chunk = map_chunk_by_coords[std::pair<float, float>(chunk_loc.x, chunk_loc.y)];
    return chunk;
}

MapChunk* LocationHandler::getNearestPathableChunk(sc2::Point2D loc_) {
    float min_dist = std::numeric_limits<float>::max();
    MapChunk* nearest = nullptr;

    for (auto c : map_chunks) {
        if (c->isPathable()) {
            float dist = sc2::DistanceSquared2D(loc_, c->getLocation());
            if (dist < min_dist) {
                min_dist = dist;
                nearest = c;
            }
        }
    }
    return nearest;
}

sc2::Point2D LocationHandler::getProxyLocation() {
    return proxy_location;
}

sc2::Point2D LocationHandler::getStartLocation()
{
    return start_location;
}

sc2::Point2D LocationHandler::getCenterPathableLocation() {
    if (!chunks_initialized)
        return INVALID_POINT;

    if (center_chunk == nullptr) {
        center_chunk = getNearestPathableChunk(map_center);
    }
    return center_chunk->getLocation();
}

sc2::Point2D LocationHandler::getRallyPointTowardsThreat()
// calculate a pathable point 2/3 of the way towards the location to send the attack
{
    sc2::Point2D p1 = start_location;
    sc2::Point2D p2 = smartPriorityAttack();
    sc2::Point2D p3 = NO_POINT_FOUND;

    p3 = ((p2 - p1) * 2 / 3) + p1;
    MapChunk* best_chunk = getNearestPathableChunk(p3);
    if (best_chunk == nullptr) {
        return NO_POINT_FOUND;
    }
    return best_chunk->getLocation();
}

void LocationHandler::setHighestThreat(double threat_)
{
    highest_threat = threat_;
}

double LocationHandler::getHighestThreat()
{
    return highest_threat;
}

void LocationHandler::setHighestThreatChunk(MapChunk* chunk_)
{
    high_threat_chunk = chunk_;
}

MapChunk* LocationHandler::getHighestThreatChunk()
{
    if (high_threat_chunk == nullptr) {
        assert(!enemy_start_locations.empty());
        sc2::Point2D clockwise_enemy = getEnemyStartLocationByIndex(0);
        assert(chunks_initialized);
        MapChunk* chunk_ = getNearestChunk(clockwise_enemy);
        high_threat_chunk = chunk_;
        if (high_threat_chunk->getThreat() == 0) {
            high_threat_chunk->setThreat(1);
        }
    }
    return high_threat_chunk;
}

void LocationHandler::setHighestThreatAwayFromStart(double threat_)
{
    highest_threat_away_from_start = threat_;
}

double LocationHandler::getHighestThreatAwayFromStart()
{
    return highest_threat_away_from_start;
}

void LocationHandler::setHighestThreatChunkAwayFromStart(MapChunk* chunk_)
{
    high_threat_chunk_away_from_start = chunk_;
}

MapChunk* LocationHandler::getHighestThreatChunkAwayFromStart()
{
    if (high_threat_chunk_away_from_start == nullptr) {
        assert(!enemy_start_locations.empty());
        sc2::Point2D clockwise_enemy = getEnemyStartLocationByIndex(0);
        assert(chunks_initialized);
        MapChunk* chunk_ = getNearestChunk(clockwise_enemy);
        high_threat_chunk_away_from_start = chunk_;
        if (high_threat_chunk_away_from_start->getThreat() == 0) {
            high_threat_chunk_away_from_start->setThreat(1);
        }
    }
    return high_threat_chunk_away_from_start;
}


void LocationHandler::setHighestPathableThreat(double threat_)
{
    highest_pathable_threat = threat_;
}

double LocationHandler::getHighestPathableThreat()
{
    return highest_pathable_threat;
}

void LocationHandler::setHighestPathableThreatChunk(MapChunk* chunk_)
{
    high_threat_pathable_chunk = chunk_;
}

MapChunk* LocationHandler::getHighestPathableThreatChunk()
{
    if (high_threat_pathable_chunk == nullptr) {
        assert(!enemy_start_locations.empty());
        sc2::Point2D clockwise_enemy = getEnemyStartLocationByIndex(0);
        assert(chunks_initialized);
        MapChunk* chunk_ = getNearestPathableChunk(clockwise_enemy);
        high_threat_pathable_chunk = chunk_;
        if (high_threat_pathable_chunk->getThreat() == 0) {
            high_threat_pathable_chunk->setThreat(1);
        }
    }
    return high_threat_pathable_chunk;
}

void LocationHandler::setHighestPathableThreatAwayFromStart(double threat_)
{
    highest_pathable_threat_away_from_start = threat_;
}

double LocationHandler::getHighestPathableThreatAwayFromStart()
{
    return highest_pathable_threat_away_from_start;
}

void LocationHandler::setHighestPathableThreatChunkAwayFromStart(MapChunk* chunk_)
{
    high_threat_pathable_chunk_away_from_start = chunk_;
}

MapChunk* LocationHandler::getHighestPathableThreatChunkAwayFromStart()
{
    if (high_threat_pathable_chunk_away_from_start == nullptr) {
        assert(!enemy_start_locations.empty());
        sc2::Point2D clockwise_enemy = getEnemyStartLocationByIndex(0);
        assert(chunks_initialized);
        MapChunk* chunk_ = getNearestPathableChunk(clockwise_enemy);
        high_threat_pathable_chunk_away_from_start = chunk_;
        if (high_threat_pathable_chunk_away_from_start->getThreat() == 0) {
            high_threat_pathable_chunk_away_from_start->setThreat(1);
        }
    }
    return high_threat_pathable_chunk_away_from_start;
}

bool LocationHandler::PathableThreatExistsNearLocation(sc2::Point2D loc_, float range_) {
    // check if a threat exists near location

    MapChunk* hi_threat = getHighestPathableThreatChunkNearLocation(loc_, range_);
    if (hi_threat == nullptr)
        return false;
    double threat_amt = hi_threat->getThreat();
    if (threat_amt <= 0) {
        return false;
    }
    return true;
}

MapChunk* LocationHandler::getHighestPathableThreatChunkNearLocation(sc2::Point2D loc_, float range_) {

    // determine boundaries of chunks so we don't search the entire map

    float min_x = loc_.x - range_;
    float max_x = loc_.x + range_;
    float min_y = loc_.y - range_;
    float max_y = loc_.y + range_;

    float sq_dist = range_ * range_;

    size_t i1 = 0; // min row
    size_t i2 = 0; // max row
    size_t j1 = 0; // min col
    size_t j2 = 0; // max col

    for (float x = chunk_min_x; x < max_x && x < chunk_max_x; x += chunk_spread) { 
        if (x < min_x) {
            i1++;
        }
        i2++;
    }
    for (float y = chunk_min_y; y < max_y && y < chunk_max_y; y += chunk_spread) {
        if (y < min_y) {
            j1++;
        }
        j2++;
    }

    // iterate through chunks to determine the highest threat.
    // Index of chunk in storage (by row and col) = j * chunk_rows + i
    double highest_threat = 0;
    MapChunk* hi_chunk = nullptr;

    assert(map_chunk_storage.size() > (j2 * chunk_rows + i2));

    for (size_t j = j1; j <= j2; j++) {
        for (size_t i = i1; i <= i2; i++) {
            MapChunk* chunk = map_chunk_storage[j * chunk_rows + i].get();
            if (chunk->isPathable()) {
                if (sc2::DistanceSquared2D(chunk->getLocation(), loc_) <= sq_dist) {
                    if (chunk->getThreat() > highest_threat) {
                        hi_chunk = chunk;
                        highest_threat = chunk->getThreat();
                    }
                }
            }
        }
    }
    return hi_chunk;
}

sc2::Point2D LocationHandler::getThreatNearStart() {
    MapChunk* chunk = getHighestPathableThreatChunkNearLocation(start_location, 50.0F);
    if (chunk == nullptr) {
        return NO_POINT_FOUND;
    }
    return chunk->getLocation();
}

float LocationHandler::distSquaredFromStart(sc2::Point2D loc_) {
    if (!bases.empty()) {
        return (sc2::DistanceSquared2D(bases[0].getTownhall(), loc_));
    }
    else {
        return(sc2::DistanceSquared2D(agent->Observation()->GetStartLocation(), loc_));
    }
}

bool LocationHandler::chunksInitialized() {
    return chunks_initialized;
}

void LocationHandler::setProxyLocation(sc2::Point2D location_) {
    proxy_location = location_;
}

void LocationHandler::initSetStartLocation()
{
    start_location = agent->Observation()->GetStartLocation();
}

sc2::Point2D LocationHandler::getEnemyStartLocationByIndex(int index_) {
    // it is important to ensure this still functions with indices out of range
    // as Strategy source files may not be able to know the number of indices

    int num_locs = agent->Observation()->GetGameInfo().start_locations.size();
    return enemy_start_locations.at(index_ % num_locs);
}

sc2::Point2D LocationHandler::getBestEnemyLocation() {
    return enemy_start_locations.at(enemy_start_location_index);
}
