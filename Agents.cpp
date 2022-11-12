#include "Agents.h"
#include "Directive.h"
#include "Squad.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2api/sc2_client.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

class TriggerCondition;
class SquadMember;

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

SquadMember* BotAgent::getSquadMember(const sc2::Unit& unit) {
	assert(unit.alliance == sc2::Unit::Alliance::Self); // only call this on allied units!

	// get the SquadMember object for a given sc2::Unit object
	for (SquadMember* s : squad_members) {
		if (&(s->unit) == &unit) {
			return s;
		}
	}
	return nullptr; // handle erroneous case where unit has no squad
}

std::vector<SquadMember*> BotAgent::getIdleWorkers() {
	// get all worker units with no active orders
	std::vector<SquadMember*> idle_workers;
	
	std::vector<SquadMember*> workers = filter_by_flag(squad_members, FLAGS::IS_WORKER);
	for (SquadMember* s : workers) {
		if (s->is_idle()) {
			idle_workers.push_back(s);
		}
	}
	
	return idle_workers;
}

void BotAgent::initStartingUnits() {
	// add all starting units to their respective squads
	const sc2::ObservationInterface* observation = Observation();
	sc2::Units units = observation->GetUnits(sc2::Unit::Alliance::Self);
	for (const sc2::Unit* u : units) {
		sc2::UNIT_TYPEID u_type = u->unit_type;
		if (u_type == sc2::UNIT_TYPEID::TERRAN_SCV ||
			u_type == sc2::UNIT_TYPEID::ZERG_DRONE ||
			u_type == sc2::UNIT_TYPEID::PROTOSS_PROBE) 
		{
			SquadMember* worker = new SquadMember(*u, SQUAD::SQUAD_WORKER);
			Directive directive_get_minerals_near_Base(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION, start_location);
			worker->assignDirective(directive_get_minerals_near_Base);
			squad_members.push_back(worker);
		}
		if (u_type == sc2::UNIT_TYPEID::PROTOSS_NEXUS ||
			u_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
			u_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING ||
			u_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
			u_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING ||
			u_type == sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS ||
			u_type == sc2::UNIT_TYPEID::ZERG_HATCHERY ||
			u_type == sc2::UNIT_TYPEID::ZERG_HIVE ||
			u_type == sc2::UNIT_TYPEID::ZERG_LAIR) {
			SquadMember* townhall = new SquadMember(*u, SQUAD::SQUAD_TOWNHALL);
			squad_members.push_back(townhall);
		}
	}
}

void BotAgent::OnGameStart() {
	start_location = Observation()->GetStartLocation();
	BotAgent::initVariables();
	BotAgent::initStartingUnits();
	std::cout << "Start Location: " << start_location.x << "," << start_location.y << std::endl;

	// How to add a new StrategyOrder to the bot's portfolio:
	// Create a StrategyOrder, Create a Directive, set the StrategyOrder directive, add TriggerCondition(s), push_back into strategies vector
	StrategyOrder build_pylon_with_100_minerals(this);
	Directive directive_build_pylon_at_base(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, start_location);
	build_pylon_with_100_minerals.setDirective(directive_build_pylon_at_base);
	build_pylon_with_100_minerals.addTriggerCondition(COND::MIN_MINERALS, 100);
	build_pylon_with_100_minerals.addTriggerCondition(COND::MAX_FOOD, 4);
	strategies.push_back(build_pylon_with_100_minerals);

	StrategyOrder train_probe_with_50_minerals(this);
	Directive train_probe(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
	train_probe_with_50_minerals.setDirective(train_probe);
	train_probe_with_50_minerals.addTriggerCondition(COND::MIN_MINERALS, 50);
	train_probe_with_50_minerals.addTriggerCondition(COND::MAX_UNIT_OF_TYPE, 15, sc2::UNIT_TYPEID::PROTOSS_PROBE);
	strategies.push_back(train_probe_with_50_minerals);

	StrategyOrder build_forge_with_150_minerals(this);
	Directive directive_build_forge_at_base(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_FORGE, start_location);
	build_forge_with_150_minerals.setDirective(directive_build_forge_at_base);
	build_forge_with_150_minerals.addTriggerCondition(COND::MIN_MINERALS, 150);
	build_forge_with_150_minerals.addTriggerCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
	strategies.push_back(build_forge_with_150_minerals);

	StrategyOrder build_cannon_with_150_minerals(this);
	Directive directive_build_photon_cannon(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, start_location);
	build_cannon_with_150_minerals.setDirective(directive_build_photon_cannon);
	build_cannon_with_150_minerals.addTriggerCondition(COND::MIN_MINERALS, 150);
	build_cannon_with_150_minerals.addTriggerCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
	strategies.push_back(build_cannon_with_150_minerals);
}

void::BotAgent::OnStep_100() {
	// occurs every 100 steps
	const sc2::ObservationInterface* observation = Observation();
	std::vector<SquadMember*> idle_workers = getIdleWorkers();
	for (SquadMember* s: idle_workers) {
		s->executeDefaultDirective(this, observation);
	}
}

void::BotAgent::OnStep_1000() {
	// occurs every 1000 steps
	std::cout << ".";
}

void BotAgent::OnStep() {
	const sc2::ObservationInterface* observation = Observation();
	int minerals = observation->GetMinerals();
	int gameloop = observation->GetGameLoop();
	if (gameloop % 100 == 0) {
		OnStep_100();
	}
	if (gameloop % 1000 == 0) {
		OnStep_1000();
	}

	for (StrategyOrder s : strategies) {
		if (s.checkTriggerConditions(observation)) {
			s.execute(observation);
		}
	}
}

//void BotAgent::OnUnitIdle(const sc2::Unit* unit) {
	// do stuff
//}

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

const sc2::Unit* BotAgent::FindNearestMineralPatch(sc2::Point2D location) {
	const sc2::ObservationInterface* obs = Observation();
	sc2::Units units = sc2::Client::Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target = nullptr;
	for (const auto& u : units) {
		sc2::UNIT_TYPEID type_ = u->unit_type;
		if (type_ == sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750 ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750 ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750 ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750 ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750 ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750 ||
			type_ == sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD) {
			float d = sc2::DistanceSquared2D(u->pos, location);		
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	return target;
}

std::vector<SquadMember*> BotAgent::filter_by_flag(std::vector<SquadMember*> squad_vector, FLAGS flag) {
	// filter a vector of SquadMember* by the given flag
	std::vector<SquadMember*> filtered_squad;
	
	std::copy_if(squad_vector.begin(), squad_vector.end(), std::back_inserter(filtered_squad),
		[flag](SquadMember* s) { return s->has_flag(flag); });
	
	return filtered_squad;
}

std::vector<SquadMember*> BotAgent::filter_by_flags(std::vector<SquadMember*> squad_vector, std::vector<FLAGS> flag_list) {
	// filter a vector of SquadMember* by several flags
	std::vector<SquadMember*> filtered_squad;
	for (FLAGS f : flag_list) {
		filtered_squad = filter_by_flag(filtered_squad, f);
	}
	return filtered_squad;
}