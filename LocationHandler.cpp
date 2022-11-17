#pragma once
#include "sc2api/sc2_api.h"
#include "Agents.h"
#include "LocationHandler.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"


LocationHandler::LocationHandler(BotAgent* agent_){
    agent = agent_;
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
    for (int i = 0; i < agent->bases.size(); i++) {
        float b_dist = sc2::DistanceSquared2D(agent->bases[i].get_townhall(), location_);
        if (b_dist < distance) {
            lowest_index = i;
            distance = b_dist;
        }
    }
    return lowest_index;
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
        if (location == sc2::Point2D(158.5, 158.5)) {
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
    switch (map_index) {
    case 1:
        switch (p_id) {
        case 1:
        {
            Base main_base(observation->GetStartLocation());
            Base exp_1(66.5, 161.5);
            agent->bases.push_back(exp_1);

            Base exp_2(54.5, 132.5);
            agent->bases.push_back(exp_2);

            Base exp_3(45.5, 20.5);
            agent->bases.push_back(exp_3);

            Base exp_4(93.5, 156.5);
            agent->bases.push_back(exp_4);

            Base exp_5(35.5, 93.5);
            agent->bases.push_back(exp_5);

            Base exp_6(30.5, 66.5);
            agent->bases.push_back(exp_6);

            Base exp_7(132.5, 137.5);
            agent->bases.push_back(exp_7);

            Base exp_8(59.5, 54.5);
            agent->bases.push_back(exp_8);

            Base exp_9(161.5, 125.5);
            agent->bases.push_back(exp_9);

            Base exp_10(156.5, 98.5);
            agent->bases.push_back(exp_10);

            Base exp_11(98.5, 35.5);
            agent->bases.push_back(exp_11);

            Base exp_12(125.5, 30.5);
            agent->bases.push_back(exp_12);

            Base exp_13(137.5, 59.5);
            agent->bases.push_back(exp_13);
            break;
        }
        break;
        }
    case 2:
        switch (p_id) {
        case 1:
        {
            agent->enemy_location = sc2::Point2D(114.5, 25.5);
            Base main_base(observation->GetStartLocation());
            agent->bases.push_back(main_base);

            Base exp_1(82.5, 23.5);
            agent->bases.push_back(exp_1);

            Base exp_2(115.5, 63.5);
            agent->bases.push_back(exp_2);

            Base exp_3(45.5, 20.5);
            agent->bases.push_back(exp_3);

            Base exp_4(120.5, 104.5);
            agent->bases.push_back(exp_4);

            Base exp_5(98.5, 138.5);
            agent->bases.push_back(exp_5);

            Base exp_6(23.5, 55.5);
            agent->bases.push_back(exp_6);

            Base exp_7(28.5, 96.5);
            agent->bases.push_back(exp_7);

            Base exp_8(61.5, 136.5);
            agent->bases.push_back(exp_8);
            break;
        }
        case 2:
        {
            agent->enemy_location = sc2::Point2D(29.5, 134.5);
            Base main_base(observation->GetStartLocation());
            agent->bases.push_back(main_base);

            Base exp_1(61.5, 136.5);
            agent->bases.push_back(exp_1);

            Base exp_2(28.5, 96.5);
            agent->bases.push_back(exp_2);

            Base exp_3(23.5, 55.5);
            agent->bases.push_back(exp_3);

            Base exp_4(98.5, 138.5);
            agent->bases.push_back(exp_4);

            Base exp_5(120.5, 104.5);
            agent->bases.push_back(exp_5);

            Base exp_6(45.5, 20.5);
            agent->bases.push_back(exp_6);

            Base exp_7(115.5, 63.5);
            agent->bases.push_back(exp_7);

            Base exp_8(82.5, 23.5);
            agent->bases.push_back(exp_8);
            break;
        }
        break;
        }
    case 3:
        switch (p_id) {
        case 1:
        {
            agent->proxy_location = sc2::Point2D(28.0, 56.0);
            agent->enemy_location = sc2::Point2D(62.5, 28.5);

            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(146.0, 128.0);
            main_base.add_build_area(132.0, 132.0);
            main_base.add_defend_point(149.0, 120.0);
            main_base.set_active();
            agent->bases.push_back(main_base);

            Base exp_1(164.5, 140.5);
            exp_1.add_build_area(165.0, 135.0);
            exp_1.add_defend_point(168.0, 132.0);
            agent->bases.push_back(exp_1);

            Base exp_2(149.5, 102.5);
            exp_2.add_build_area(144.0, 106.0);
            exp_2.add_defend_point(141.0, 95.0);
            agent->bases.push_back(exp_2);

            Base exp_3(166.5, 69.5);
            exp_3.add_build_area(161.0, 79.0);
            exp_3.add_defend_point(155.0, 69.0);
            agent->bases.push_back(exp_3);

            Base exp_4(119.5, 111.5);
            exp_4.add_defend_point(115.0, 105.0);
            exp_4.add_defend_point(110.0, 115.0);
            agent->bases.push_back(exp_4);

            Base exp_5(127.5, 57.5);
            exp_5.add_defend_point(122.0, 69.0);
            exp_5.add_build_area(137.0, 54.0);
            agent->bases.push_back(exp_5);

            Base exp_6(165.5, 23.5);
            exp_6.add_defend_point(157.0, 23.0);
            agent->bases.push_back(exp_6);

            Base exp_7(93.5, 147.5);
            exp_7.add_defend_point(94.0, 141.0);
            agent->bases.push_back(exp_7);

            Base exp_8(106.5, 20.5);
            exp_8.add_defend_point(106.0, 27.0);
            agent->bases.push_back(exp_8);

            Base exp_9(34.5, 144.5);
            exp_9.add_defend_point(43.0, 145.0);
            agent->bases.push_back(exp_9);

            Base exp_10(72.5, 110.5);
            exp_10.add_defend_point(78.0, 99.0);
            exp_10.add_defend_point(63.0, 114.0);
            agent->bases.push_back(exp_10);

            Base exp_11(80.5, 56.5);
            exp_11.add_defend_point(85.0, 63.0);
            exp_11.add_defend_point(90.0, 53.0);
            agent->bases.push_back(exp_11);

            Base exp_12(33.5, 98.5);
            exp_12.add_build_area(39.0, 89.0);
            exp_12.add_defend_point(45.0, 99.0);
            agent->bases.push_back(exp_12);

            Base exp_13(50.5, 65.5);
            exp_13.add_build_area(56.0, 62.0);
            exp_13.add_defend_point(59.0, 73.0);
            agent->bases.push_back(exp_13);

            Base exp_14(35.5, 27.5);
            exp_14.add_build_area(35.0, 33.0);
            exp_14.add_defend_point(32.0, 36.0);
            agent->bases.push_back(exp_14);

            break;
        }
        case 2:
        {
            agent->proxy_location = sc2::Point2D(172.0, 112.0);
            agent->enemy_location = sc2::Point2D(137.5, 139.5);
            Base main_base(observation->GetStartLocation());
            main_base.add_build_area(54.0, 40.0);
            main_base.add_build_area(68.0, 36.0);
            main_base.add_defend_point(51.0, 48.0);
            main_base.set_active();
            agent->bases.push_back(main_base);

            Base exp_1(35.5, 27.5);
            exp_1.add_build_area(35.0, 33.0);
            exp_1.add_defend_point(32.0, 36.0);
            agent->bases.push_back(exp_1);

            Base exp_2(50.5, 65.5);
            exp_2.add_build_area(56.0, 62.0);
            exp_2.add_defend_point(59.0, 73.0);
            agent->bases.push_back(exp_2);

            Base exp_3(33.5, 98.5);
            exp_3.add_build_area(39.0, 89.0);
            exp_3.add_defend_point(45.0, 99.0);
            agent->bases.push_back(exp_3);

            Base exp_4(80.5, 56.5);
            exp_4.add_defend_point(85.0, 63.0);
            exp_4.add_defend_point(90.0, 53.0);
            agent->bases.push_back(exp_4);

            Base exp_5(72.5, 110.5);
            exp_5.add_defend_point(78.0, 99.0);
            exp_5.add_defend_point(63.0, 114.0);
            agent->bases.push_back(exp_5);

            Base exp_6(34.5, 144.5);
            exp_6.add_defend_point(43.0, 145.0);
            agent->bases.push_back(exp_6);

            Base exp_7(106.5, 20.5);
            exp_7.add_defend_point(106.0, 27.0);
            agent->bases.push_back(exp_7);

            Base exp_8(93.5, 147.5);
            exp_8.add_defend_point(94.0, 141.0);
            agent->bases.push_back(exp_8);

            Base exp_9(165.5, 23.5);
            exp_9.add_defend_point(157.0, 23.0);
            agent->bases.push_back(exp_9);

            Base exp_10(127.5, 57.5);
            exp_10.add_defend_point(122.0, 69.0);
            exp_10.add_build_area(137.0, 54.0);
            agent->bases.push_back(exp_10);

            Base exp_11(119.5, 111.5);
            exp_11.add_defend_point(115.0, 105.0);
            exp_11.add_defend_point(110.0, 115.0);
            agent->bases.push_back(exp_11);

            Base exp_12(166.5, 69.5);
            exp_12.add_build_area(161.0, 79.0);
            exp_12.add_defend_point(155.0, 69.0);
            agent->bases.push_back(exp_12);

            Base exp_13(149.5, 102.5);
            exp_13.add_build_area(144.0, 106.0);
            exp_13.add_defend_point(141.0, 95.0);
            agent->bases.push_back(exp_13);

            Base exp_14(164.5, 140.5);
            exp_14.add_build_area(165.0, 135.0);
            exp_14.add_defend_point(168.0, 132.0);
            agent->bases.push_back(exp_14);

            break;
        }
        break;
        }
    }
}
