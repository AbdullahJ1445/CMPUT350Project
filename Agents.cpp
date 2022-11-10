#include "Agents.h"
#include "Triggers.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2api/sc2_client.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"


void BotAgent::initVariables() {
	const sc2::ObservationInterface* observation = Observation();
	map_name = observation->GetGameInfo().map_name;

	if (map_name == "Proxima Station LE")
		map_index = 3; else
		if (map_name == "Bel'Shir Vestige LE (Void)" || map_name == "Bel'Shir Vestige LE")
			map_index = 2; else
			if (map_name == "Cactus Valley LE")
				map_index = 1; else
				map_index = 0;

	player_start_id = getPlayerIDForMap(map_index, observation->GetStartLocation());
	initLocations(map_index, player_start_id);

	std::cout << "Map Name: " << map_name << std::endl;
	std::cout << "Player Start ID: " << player_start_id << std::endl;
}

void BotAgent::OnGameStart() {
	BotAgent::initVariables();
	start_location = Observation()->GetStartLocation();
	StrategyOrder build_pylon_with_500_minerals(this, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, sc2::Point2D(0, 0), sc2::Point2D(0, 0));
	build_pylon_with_500_minerals.addTriggerCondition(MIN_MINERALS, 500);
	strategies.push_back(build_pylon_with_500_minerals);
}

void BotAgent::OnStep() {
	const sc2::ObservationInterface* observation = Observation();
	int minerals = observation->GetMinerals();
	if (observation->GetGameLoop() % 1000 == 0) {
		std::cout << ".";
	}
	for (StrategyOrder s : strategies) {
		if (s.checkTriggerConditions(observation))
			s.execute(observation);
	}
}

void BotAgent::OnUnitIdle(const sc2::Unit* unit) {
	// do stuff
}

void BotAgent::initLocations(int map_index, int p_id) {
	switch (map_index) {
	case 1:
		break;
	case 2:
		break;
	case 3:
		switch (p_id) {
		case 1:
			choke_point_1 = sc2::Point2D(146.0, 119.0);
			choke_point_2 = sc2::Point2D(137.0, 90.0);
			proxy_location = sc2::Point2D(28.0, 56.0);
			enemy_location = sc2::Point2D(62.5, 28.5);
			break;
		case 2:
			choke_point_1 = sc2::Point2D(54.0, 49.0);
			choke_point_2 = sc2::Point2D(63.0, 78.0);
			proxy_location = sc2::Point2D(172.0, 112.0);
			enemy_location = sc2::Point2D(137.5, 139.5);
			break;
		}
		break;
	}
}

int BotAgent::getPlayerIDForMap(int map_index, sc2::Point2D location) {
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

sc2::Point2D BotAgent::getNearestStartLocation(sc2::Point2D spot) {
	// Get the nearest Start Location from a given point
	float nearest_distance = 10000.0f;
	sc2::Point2D nearest_point;

	for (auto& iter : sc2::Client::Observation()->GetGameInfo().start_locations) {
		float dist = sc2::Distance2D(spot, iter);
		if (dist <= nearest_distance) {
			nearest_distance = dist;
			nearest_point = iter;
		}
	}
	return nearest_point;
}