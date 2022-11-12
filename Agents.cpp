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
	
	StrategyOrder build_pylon_at_choke(this);
	Directive directive_build_pylon_at_choke(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, choke_point_1);
	build_pylon_at_choke.setDirective(directive_build_pylon_at_choke);
	build_pylon_at_choke.addTriggerCondition(COND::MIN_MINERALS, 100);
	build_pylon_at_choke.addTriggerCondition(COND::MAX_FOOD, 6);
	build_pylon_at_choke.addTriggerCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, start_location);
	build_pylon_at_choke.addTriggerCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, choke_point_1);
	strategies.push_back(build_pylon_at_choke);

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
	build_cannon_with_150_minerals.addTriggerCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 4, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, start_location);
	strategies.push_back(build_cannon_with_150_minerals);

	StrategyOrder build_cannon_at_choke(this);
	Directive directive_build_cannon_at_choke(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, choke_point_1);
	build_cannon_at_choke.setDirective(directive_build_cannon_at_choke);
	build_cannon_at_choke.addTriggerCondition(COND::MIN_MINERALS, 150);
	build_cannon_at_choke.addTriggerCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
	build_cannon_at_choke.addTriggerCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, choke_point_1);
	strategies.push_back(build_cannon_at_choke);
	
	StrategyOrder build_assimilator_with_75_minerals(this);
	Directive directive_build_assimilator(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, start_location);
	build_assimilator_with_75_minerals.setDirective(directive_build_assimilator);
	build_assimilator_with_75_minerals.addTriggerCondition(COND::MIN_MINERALS, 75);
	build_assimilator_with_75_minerals.addTriggerCondition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
	strategies.push_back(build_assimilator_with_75_minerals);

	StrategyOrder build_cybernetics_core_with_200_minerals(this);
	Directive directive_build_cybernetics(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_CYBERNETICSCORE, start_location);
	build_cybernetics_core_with_200_minerals.setDirective(directive_build_cybernetics);
	build_cybernetics_core_with_200_minerals.addTriggerCondition(COND::MIN_MINERALS, 200);
	build_cybernetics_core_with_200_minerals.addTriggerCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
	strategies.push_back(build_cybernetics_core_with_200_minerals);

	StrategyOrder build_gateway_with_150_minerals(this);
	Directive directive_build_gateway(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, start_location);
	build_gateway_with_150_minerals.setDirective(directive_build_gateway);
	build_gateway_with_150_minerals.addTriggerCondition(COND::MIN_MINERALS, 150);
	build_gateway_with_150_minerals.addTriggerCondition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
	strategies.push_back(build_gateway_with_150_minerals);

	StrategyOrder build_twilight_council_150_minerals_100_gas(this);
	Directive directive_twilight_council(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_TWILIGHTCOUNCIL, start_location);
	build_twilight_council_150_minerals_100_gas.setDirective(directive_twilight_council);
	build_twilight_council_150_minerals_100_gas.addTriggerCondition(COND::MIN_MINERALS, 150);
	build_twilight_council_150_minerals_100_gas.addTriggerCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
	strategies.push_back(build_twilight_council_150_minerals_100_gas);

	StrategyOrder train_stalker_125_minerals_50_gas(this);
	Directive directive_train_stalker(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_STALKER);
	train_stalker_125_minerals_50_gas.setDirective(directive_train_stalker);
	train_stalker_125_minerals_50_gas.addTriggerCondition(COND::MIN_MINERALS, 125);
	train_stalker_125_minerals_50_gas.addTriggerCondition(COND::MIN_GAS, 50);
	train_stalker_125_minerals_50_gas.addTriggerCondition(COND::MIN_FOOD, 2);
	strategies.push_back(train_stalker_125_minerals_50_gas);

	StrategyOrder research_blink_150_150(this);
	Directive directive_research_blink(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, sc2::ABILITY_ID::RESEARCH_BLINK);
	research_blink_150_150.setDirective(directive_research_blink);
	research_blink_150_150.addTriggerCondition(COND::MIN_MINERALS, 150);
	research_blink_150_150.addTriggerCondition(COND::MIN_GAS, 150);
	research_blink_150_150.addTriggerCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
	strategies.push_back(research_blink_150_150);
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

void BotAgent::OnBuildingConstructionComplete(const sc2::Unit* unit) {
	SquadMember* structure = new SquadMember(*unit, SQUAD::SQUAD_STRUCTURE);
	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_NEXUS ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
		unit_type == sc2::UNIT_TYPEID::ZERG_HATCHERY) {
		structure->flags.insert(FLAGS::IS_TOWNHALL);
		structure->flags.insert(FLAGS::IS_SUPPLY);
	}
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_BUNKER ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_MISSILETURRET ||
		unit_type == sc2::UNIT_TYPEID::ZERG_SPINECRAWLER ||
		unit_type == sc2::UNIT_TYPEID::ZERG_SPORECRAWLER) {
		structure->flags.insert(FLAGS::IS_DEFENSE);
	}
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY ||
		unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR ||
		unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERYRICH ||
		unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTORRICH) {
		// assign 3 workers to mine this
		AssignNearbyWorkerToGasStructure(*unit);
		AssignNearbyWorkerToGasStructure(*unit);
		AssignNearbyWorkerToGasStructure(*unit);
	}
	squad_members.push_back(structure);
}

void BotAgent::OnUnitCreated(const sc2::Unit* unit) {
	if (getSquadMember(*unit)) {
		return;
	}
	SquadMember* new_squad;
	bool squad_created = false;
	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	if (unit_type == sc2::UNIT_TYPEID::TERRAN_SCV |
		unit_type == sc2::UNIT_TYPEID::ZERG_DRONE |
		unit_type == sc2::UNIT_TYPEID::PROTOSS_PROBE) {
		new_squad = new SquadMember(*unit, SQUAD::SQUAD_WORKER);
		squad_created = true;
	}
	if (squad_created)
		squad_members.push_back(new_squad);
}

bool BotAgent::AssignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure) {
	std::vector<FLAGS> flags;
	bool found_viable_unit = false;
	//flags.push_back(FLAGS::IS_WORKER);
	//flags.push_back(FLAGS::IS_MINERAL_GATHERER);
	//std::vector<SquadMember*> worker_miners = filter_by_flags(squad_members, flags);
	std::vector<SquadMember*> worker_miners = filter_by_flag(squad_members, FLAGS::IS_MINERAL_GATHERER);
	float distance = std::numeric_limits<float>::max();
	SquadMember* target = nullptr;
	for (SquadMember* s : worker_miners) {
		float d = sc2::DistanceSquared2D(s->unit.pos, gas_structure.pos);
		if (d < distance) {
			distance = d;
			target = s;
			found_viable_unit = true;
		}
	}
	if (found_viable_unit) {
		target->flags.erase(FLAGS::IS_MINERAL_GATHERER);
		target->flags.insert(FLAGS::IS_GAS_GATHERER);
		// make the unit continue to mine gas after being idle
		Directive directive_get_gas(Directive::DEFAULT_DIRECTIVE, Directive::GET_GAS_NEAR_LOCATION, gas_structure.pos);
		target->assignDirective(directive_get_gas);
		Actions()->UnitCommand(&(target->unit), sc2::ABILITY_ID::HARVEST_GATHER, &gas_structure);

		
		return true;
	}
	return false;
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

bool isMineralPatch(const sc2::Unit* unit_) {
	// check whether a given unit is a mineral patch
	sc2::UNIT_TYPEID type_ = unit_->unit_type;
	return (type_ == sc2::UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750 ||
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
		type_ == sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD);
}

bool isGeyser(const sc2::Unit* unit_) {
	// check whether a given unit is a geyser
	sc2::UNIT_TYPEID type_ = unit_->unit_type;
	return (type_ == sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
}

const sc2::Unit* BotAgent::FindNearestMineralPatch(sc2::Point2D location) {
	const sc2::ObservationInterface* obs = Observation();
	sc2::Units units = sc2::Client::Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit * target = nullptr;
	for (const auto& u : units) {
		if (isMineralPatch(u)) {
			float d = sc2::DistanceSquared2D(u->pos, location);		
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	return target;
}

const sc2::Unit* BotAgent::FindNearestGeyser(sc2::Point2D location) {
	const sc2::ObservationInterface* obs = Observation();
	sc2::Units units = sc2::Client::Observation()->GetUnits(sc2::Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const sc2::Unit* target = nullptr;
	for (const auto& u : units) {
		if (isGeyser(u)) {
			float d = sc2::DistanceSquared2D(u->pos, location);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	return target;
}

const sc2::Unit* BotAgent::FindNearestGasStructure(sc2::Point2D location) {
	const sc2::ObservationInterface* obs = Observation();
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