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

    float d_threat = agent_->getValue(unit) * modifier;
    threat += d_threat;
}


LocationHandler::LocationHandler(BasicSc2Bot* agent_){
    agent = agent_;
    enemy_start_location_index = 0;
    chunks_initialized = false;
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
        float b_dist = sc2::DistanceSquared2D(bases[i].get_townhall(), location_);
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
        if (agent->is_mineral_patch(u)) {
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
    sc2::Units units = agent->Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const sc2::Unit* target = nullptr;
    for (const auto& u : units) {
        if (agent->is_geyser(u)) {
            float d = sc2::DistanceSquared2D(u->pos, location);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}    

const sc2::Unit* LocationHandler::getNearestGasStructure(sc2::Point2D location) {
    const sc2::ObservationInterface* obs = agent->Observation();
    sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
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

    std::unordered_set<Mob*> townhalls = agent->mobH->filter_by_flag(agent->mobH->get_mobs(), FLAGS::IS_TOWNHALL);
    for (auto m : townhalls) {
        return &(m->unit);
    }
    return nullptr;
}

sc2::Point2D LocationHandler::getHighestThreatLocation(bool pathable_) {
    std::unordered_set<MapChunk*> chunkset;

    if (pathable_)
        chunkset = pathable_map_chunks;
    else
        chunkset = map_chunks;

    float highest_threat = 0;
    int highest_id = -99;

    for (auto chunk : chunkset) {
        if (chunk->getThreat() > highest_threat) {
            highest_threat = chunk->getThreat();
            highest_id = chunk->getID();
        }
    }
    if (highest_id != -99) {
        return map_chunk_by_id[highest_id]->getLocation();
    }
    else {
        return NO_POINT_FOUND;
    }
}

MapChunk* LocationHandler::getChunkByCoords(std::pair<float, float> coords) {
    return map_chunk_by_coords[coords];
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
        //std::cout << "(unseen)";
        return getClosestUnseenLocation(pathable_);
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

    std::unordered_set<Mob*> mobs = agent->mobH->get_mobs();
    std::unordered_set<Mob*> flying_mobs = agent->mobH->filter_by_flag(mobs, FLAGS::IS_FLYING);
    
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
            closest_mob = Directive::get_closest_to_location(flying_mobs , loc);
        }
        else {
            closest_mob = Directive::get_closest_to_location(mobs, loc);
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

int LocationHandler::getPlayerIDForMap(int map_index, sc2::Point2D location) {
    location = getNearestStartLocation(location);
    int p_id = 0;
    switch (map_index) {
    case 1:
        // Cactus Valley LE
        if (location == sc2::Point2D(33.5, 158.5)) {
            // top left
            p_id = 1;
        }
        if (location == sc2::Point2D(158.5, 158.5)) {
            // top right
            p_id = 2;
        }
        if (location == sc2::Point2D(158.5, 33.5)) {
            // bottom right
            p_id = 3;
        }
        if (location == sc2::Point2D(33.5, 33.5)) {
            // bottom left
            p_id = 4;
        }
        break;
    case 2:
        // Bel'Shir Vestige LE
        if (location == sc2::Point2D(114.5, 25.5)) {
            // bottom right
            p_id = 1;
        }
        if (location == sc2::Point2D(29.5, 134.5)) {
            // top left
            p_id = 2;
        }
        break;
    case 3:
        // Proxima Station LE
        if (location == sc2::Point2D(137.5, 139.5)) {
            // top right
            p_id = 1;
        }
        if (location == sc2::Point2D(62.5, 28.5)) {
            // bottom left
            p_id = 2;
        }
        break;
    }
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

            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(32, 147);
            bases.push_back(main_base);

            Base exp_1(66.5, 161.5);
            exp_1.add_defend_point(64, 152);
            bases.push_back(exp_1); // radius 6.0F

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
        }
        else if (p_id == 2) {
            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(147, 160);
            bases.push_back(main_base);

            Base exp_1(161.5, 125.5);
            exp_1.add_defend_point(152, 128); // radius 6.0F
            bases.push_back(exp_1);
        }
        else if (p_id == 3) {
            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(128, 40);
            bases.push_back(main_base);

            Base exp_1(125.5, 30.5);
            exp_1.add_defend_point(128, 40); // radius 6.0F
            bases.push_back(exp_1);
        }
        else if (p_id == 4) {
            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(45, 32);
            bases.push_back(main_base);

            Base exp_1(30.5, 66.5);
            exp_1.add_defend_point(40, 64);  // radius 6.0F
            bases.push_back(exp_1);
        }
    }

    else if (map_index == 2) {
        // BELSHIR VESTIGE

        if (p_id == 1) {
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
            bases.push_back(exp_7);

            Base exp_8(61.5, 136.5);
            bases.push_back(exp_8);

        } 
        else if (p_id == 2) {

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
            bases.push_back(exp_7);

            Base exp_8(82.5, 23.5);
            bases.push_back(exp_8);
        }
    }
    else if (map_index == 3) {
        
        // PROXIMA STATION

        if (p_id == 1) {
            setProxyLocation(sc2::Point2D(28.0, 56.0));
            initAddEnemyStartLocation(sc2::Point2D(62.5, 28.5));

            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(146.0, 128.0);
            main_base.add_build_area(132.0, 132.0);
            main_base.add_defend_point(149.0, 120.0);
            main_base.set_active();
            bases.push_back(main_base);

            Base exp_1(164.5, 140.5);
            exp_1.add_build_area(165.0, 135.0);
            exp_1.add_defend_point(168.0, 132.0);
            bases.push_back(exp_1);

            Base exp_2(149.5, 102.5);
            exp_2.add_build_area(144.0, 106.0);
            exp_2.add_defend_point(141.0, 95.0);
            bases.push_back(exp_2);

            Base exp_3(166.5, 69.5);
            exp_3.add_build_area(161.0, 79.0);
            exp_3.add_defend_point(155.0, 69.0);
            bases.push_back(exp_3);

            Base exp_4(119.5, 111.5);
            exp_4.add_defend_point(115.0, 105.0);
            exp_4.add_defend_point(110.0, 115.0);
            bases.push_back(exp_4);

            Base exp_5(127.5, 57.5);
            exp_5.add_defend_point(122.0, 69.0);
            exp_5.add_build_area(137.0, 54.0);
            bases.push_back(exp_5);

            Base exp_6(165.5, 23.5);
            exp_6.add_defend_point(157.0, 23.0);
            bases.push_back(exp_6);

            Base exp_7(93.5, 147.5);
            exp_7.add_defend_point(94.0, 141.0);
            bases.push_back(exp_7);

            Base exp_8(106.5, 20.5);
            exp_8.add_defend_point(106.0, 27.0);
            bases.push_back(exp_8);

            Base exp_9(34.5, 144.5);
            exp_9.add_defend_point(43.0, 145.0);
            bases.push_back(exp_9);

            Base exp_10(72.5, 110.5);
            exp_10.add_defend_point(78.0, 99.0);
            exp_10.add_defend_point(63.0, 114.0);
            bases.push_back(exp_10);

            Base exp_11(80.5, 56.5);
            exp_11.add_defend_point(85.0, 63.0);
            exp_11.add_defend_point(90.0, 53.0);
            bases.push_back(exp_11);

            Base exp_12(33.5, 98.5);
            exp_12.add_build_area(39.0, 89.0);
            exp_12.add_defend_point(45.0, 99.0);
            bases.push_back(exp_12);

            Base exp_13(50.5, 65.5);
            exp_13.add_build_area(56.0, 62.0);
            exp_13.add_defend_point(59.0, 73.0);
            bases.push_back(exp_13);

            Base exp_14(35.5, 27.5);
            exp_14.add_build_area(35.0, 33.0);
            exp_14.add_defend_point(32.0, 36.0);
            bases.push_back(exp_14);
        }
        else if (p_id == 2) {
            setProxyLocation(sc2::Point2D(172.0, 112.0));
            initAddEnemyStartLocation(sc2::Point2D(137.5, 139.5));
            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(54.0, 40.0);
            main_base.add_build_area(68.0, 36.0);
            main_base.add_defend_point(51.0, 48.0);
            main_base.set_active();
            bases.push_back(main_base);

            Base exp_1(35.5, 27.5);
            exp_1.add_build_area(35.0, 33.0);
            exp_1.add_defend_point(32.0, 36.0);
            bases.push_back(exp_1);

            Base exp_2(50.5, 65.5);
            exp_2.add_build_area(56.0, 62.0);
            exp_2.add_defend_point(59.0, 73.0);
            bases.push_back(exp_2);

            Base exp_3(33.5, 98.5);
            exp_3.add_build_area(39.0, 89.0);
            exp_3.add_defend_point(45.0, 99.0);
            bases.push_back(exp_3);

            Base exp_4(80.5, 56.5);
            exp_4.add_defend_point(85.0, 63.0);
            exp_4.add_defend_point(90.0, 53.0);
            bases.push_back(exp_4);

            Base exp_5(72.5, 110.5);
            exp_5.add_defend_point(78.0, 99.0);
            exp_5.add_defend_point(63.0, 114.0);
            bases.push_back(exp_5);

            Base exp_6(34.5, 144.5);
            exp_6.add_defend_point(43.0, 145.0);
            bases.push_back(exp_6);

            Base exp_7(106.5, 20.5);
            exp_7.add_defend_point(106.0, 27.0);
            bases.push_back(exp_7);

            Base exp_8(93.5, 147.5);
            exp_8.add_defend_point(94.0, 141.0);
            bases.push_back(exp_8);

            Base exp_9(165.5, 23.5);
            exp_9.add_defend_point(157.0, 23.0);
            bases.push_back(exp_9);

            Base exp_10(127.5, 57.5);
            exp_10.add_defend_point(122.0, 69.0);
            exp_10.add_build_area(137.0, 54.0);
            bases.push_back(exp_10);

            Base exp_11(119.5, 111.5);
            exp_11.add_defend_point(115.0, 105.0);
            exp_11.add_defend_point(110.0, 115.0);
            bases.push_back(exp_11);

            Base exp_12(166.5, 69.5);
            exp_12.add_build_area(161.0, 79.0);
            exp_12.add_defend_point(155.0, 69.0);
            bases.push_back(exp_12);

            Base exp_13(149.5, 102.5);
            exp_13.add_build_area(144.0, 106.0);
            exp_13.add_defend_point(141.0, 95.0);
            bases.push_back(exp_13);

            Base exp_14(164.5, 140.5);
            exp_14.add_build_area(165.0, 135.0);
            exp_14.add_defend_point(168.0, 132.0);
            bases.push_back(exp_14);
        }
    }
    initMapChunks();
    scanChunks(agent->Observation());
}

void LocationHandler::initAddEnemyStartLocation(sc2::Point2D location_) {
    enemy_start_locations.push_back(location_);
}

float LocationHandler::pathDistFromStartLocation(sc2::QueryInterface* query_, sc2::Point2D location_) {
    return query_->PathingDistance(start_location, location_);
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
    chunks_initialized = true;
    std::cout << std::endl << map_chunks.size() << " chunks initialized (" << pathable_count << " pathable)" << std::endl;
    std::cout << " minx: " << chunk_min_x << " miny: " << chunk_min_y << 
        " maxx: " << chunk_max_x << " maxy: " << chunk_max_y << 
        " spread: " << chunk_spread << " rows: " << chunk_rows << " cols: " << chunk_cols << std::endl;
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

sc2::Point2D LocationHandler::getProxyLocation() {
    return proxy_location;
}

sc2::Point2D LocationHandler::getStartLocation()
{
    return start_location;
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
