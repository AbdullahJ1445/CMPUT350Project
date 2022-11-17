#include "Agents.h"
#include "Directive.h"
#include "Mob.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2api/sc2_client.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

// forward declarations
class TriggerCondition;
class Mob;
class MobHandler;


void BotAgent::setCurrentStrategy(Strategy* strategy_) {
	// set the current strategy
	current_strategy = strategy_;
}

void BotAgent::addStrat(StrategyOrder strategy) {
	strategies.push_back(strategy);
}

bool BotAgent::AssignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure) {
	// get a nearby worker unit that is not currently assigned to gas
	// and assign it to harvest gas

	bool found_viable_unit = false;
	
	// get a filtered set of workers that are currently assigned to minerals
	std::unordered_set<FLAGS> flags{ FLAGS::IS_WORKER, FLAGS::IS_MINERAL_GATHERER };
	std::unordered_set<Mob*> worker_miners = mobH->filter_by_flags(mobH->get_mobs(), flags);

	//std::unordered_set<Mob*> worker_miners = filter_by_flag(mobs, FLAGS::IS_MINERAL_GATHERER);
	float distance = std::numeric_limits<float>::max();
	Mob* target = nullptr;
	for (Mob* m : worker_miners) {
		float d = sc2::DistanceSquared2D(m->unit.pos, gas_structure.pos);
		if (d < distance) {
			distance = d;
			target = m;
			found_viable_unit = true;
		}
	}
	if (found_viable_unit) {
		target->remove_flag(FLAGS::IS_MINERAL_GATHERER);
		target->set_flag(FLAGS::IS_GAS_GATHERER);

		// make the unit continue to mine gas after being idle
		Directive directive_get_gas(Directive::DEFAULT_DIRECTIVE, Directive::GET_GAS_NEAR_LOCATION, target->unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, gas_structure.pos);
		target->assignDefaultDirective(directive_get_gas);
		Actions()->UnitCommand(&(target->unit), sc2::ABILITY_ID::HARVEST_GATHER, &gas_structure);


		return true;
	}
	return false;
}


bool BotAgent::have_upgrade(const sc2::UpgradeID upgrade_) {
	// return true if the bot has fully researched the specified upgrade

	const sc2::ObservationInterface* observation = Observation();
	const std::vector<sc2::UpgradeID> upgrades = observation->GetUpgrades();
	return (std::find(upgrades.begin(), upgrades.end(), upgrade_) != upgrades.end());
}


bool BotAgent::can_unit_use_ability(const sc2::Unit& unit, const sc2::ABILITY_ID ability_) {
	// check if a unit is able to use a given ability

	sc2::QueryInterface* query_interface = Query();
	std::vector<sc2::AvailableAbility> abilities = (query_interface->GetAbilitiesForUnit(&unit)).abilities;
	for (auto a : abilities) {
		if (a.ability_id == ability_) {
			return true;
		}
	}
	return false;
}

bool BotAgent::is_structure(const sc2::Unit* unit) {
	// check if unit is a structure

	std::vector<sc2::Attribute> attrs = get_attributes(unit);
	return (std::find(attrs.begin(), attrs.end(), sc2::Attribute::Structure) != attrs.end());
}

bool BotAgent::is_mineral_patch(const sc2::Unit* unit_) {
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

bool BotAgent::is_geyser(const sc2::Unit* unit_) {
	// check whether a given unit is a geyser

	sc2::UNIT_TYPEID type_ = unit_->unit_type;
	return (type_ == sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
}


std::vector<sc2::Attribute> BotAgent::get_attributes(const sc2::Unit* unit) {
	// get attributes for a unit

	return getUnitTypeData(unit).attributes;
}

sc2::UnitTypeData BotAgent::getUnitTypeData(const sc2::Unit* unit) {
	// get UnitTypeData for a unit

	return Observation()->GetUnitTypeData()[unit->unit_type];
}


int BotAgent::get_index_of_closest_base(sc2::Point2D location_) {
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
	const sc2::Unit* target = nullptr;
	for (const auto& u : units) {
		if (is_mineral_patch(u)) {
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
		if (is_geyser(u)) {
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

const sc2::Unit* BotAgent::FindNearestTownhall(const sc2::Point2D location) {
	// find nearest townhall to location

	std::unordered_set<Mob*> townhalls = mobH->filter_by_flag(mobH->get_mobs(), FLAGS::IS_TOWNHALL);
	for (auto m : townhalls) {
		return &(m->unit);
	}
	return nullptr;
}

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

void BotAgent::initStartingUnits() {
	// add all starting units to their respective mobs
	const sc2::ObservationInterface* observation = Observation();
	sc2::Units units = observation->GetUnits(sc2::Unit::Alliance::Self);
	for (const sc2::Unit* u : units) {
		sc2::UNIT_TYPEID u_type = u->unit_type;
		if (u_type == sc2::UNIT_TYPEID::TERRAN_SCV ||
			u_type == sc2::UNIT_TYPEID::ZERG_DRONE ||
			u_type == sc2::UNIT_TYPEID::PROTOSS_PROBE) 
		{
			Mob worker (*u, MOB::MOB_WORKER);
			Directive directive_get_minerals_near_Base(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION, u_type, sc2::ABILITY_ID::HARVEST_GATHER, start_location);
			worker.assignDefaultDirective(directive_get_minerals_near_Base);
			mobH->addMob(worker);
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
			Mob townhall(*u, MOB::MOB_TOWNHALL);
			mobH->addMob(townhall);
		}
	}
}

void BotAgent::OnGameStart() {
	start_location = Observation()->GetStartLocation();
	mobH = new MobHandler(this); // initialize mob handler 
	BotAgent::initVariables();
	BotAgent::initStartingUnits();
	std::cout << "Start Location: " << start_location.x << "," << start_location.y << std::endl;
	std::cout << "Build Area 0: " << bases[0].get_build_area(0).x << "," << bases[0].get_build_area(0).y << std::endl;

	current_strategy->loadStrategies();
}

void::BotAgent::OnStep_100() {
	// occurs every 100 steps
}

void::BotAgent::OnStep_1000() {
	// occurs every 1000 steps
	std::cout << ".";
}

void BotAgent::OnStep() {
	const sc2::ObservationInterface* observation = Observation();
	int gameloop = observation->GetGameLoop();
	if (gameloop % 100 == 0) {
		OnStep_100();
	}
	if (gameloop % 1000 == 0) {
		OnStep_1000();
	}
	
	if (mobH->get_idle_mobs().size() > 0) {
		for (auto it = mobH->get_idle_mobs().begin(); it != mobH->get_idle_mobs().end(); ) {
			auto next = std::next(it);
			if ((*it)->hasBundledDirective()) {
				Directive bundled = (*it)->popBundledDirective();
				bundled.execute(this);
			}
			else {
				(*it)->executeDefaultDirective(this);
			}
			it = next;
		}
	}

	for (StrategyOrder s : strategies) {
		if (s.checkTriggerConditions()) {
			s.execute();
		}
	}
}

void BotAgent::OnUnitCreated(const sc2::Unit* unit) {
	const sc2::ObservationInterface* observation = Observation();
	
	// mob already exists
	if (mobH->mob_exists(*unit))
		return;

	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	MOB mob_type; // which category of mob to create

	// determine if unit is a structure
	bool structure = is_structure(unit);
	bool is_worker = false;
	int base_index = get_index_of_closest_base(unit->pos);

	if (!structure) {
		if (unit_type == sc2::UNIT_TYPEID::TERRAN_SCV |
			unit_type == sc2::UNIT_TYPEID::ZERG_DRONE |
			unit_type == sc2::UNIT_TYPEID::PROTOSS_PROBE) {
			is_worker = true;
			mob_type = MOB::MOB_WORKER;
		}
		else {
			mob_type = MOB::MOB_ARMY;
		}
	} else {
		if (unit_type == sc2::UNIT_TYPEID::PROTOSS_NEXUS ||
			unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
			unit_type == sc2::UNIT_TYPEID::ZERG_HATCHERY) {
			mob_type = MOB::MOB_TOWNHALL;
		}
		else {
			mob_type = MOB::MOB_STRUCTURE;
			if (unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR) {

				/*   this block of code is necessary to wake a protoss probe from standing idle             *|
				|*   while the assimilator is under construction.                                           *|
				|*   For some reason it does not trigger as idle after building this particular structure   */

				std::unordered_set<Mob*> gas_builders = mobH->filter_by_flag(mobH->get_mobs(), FLAGS::BUILDING_GAS);
				Mob* gas_builder = Directive::get_closest_to_location(gas_builders, unit->pos);
				gas_builder->remove_flag(FLAGS::BUILDING_GAS);
				Actions()->UnitCommand(&gas_builder->unit, sc2::ABILITY_ID::STOP);
			}
		}
	}
	Mob new_mob(*unit, mob_type);
	new_mob.set_home_location(bases[base_index].get_townhall());
	if (is_worker) {
		new_mob.set_assigned_location(new_mob.get_home_location());
		Directive directive_get_minerals_near_birth(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION,
			new_mob.unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, bases[base_index].get_townhall());
		new_mob.assignDefaultDirective(directive_get_minerals_near_birth);
	}
	else {
		if (!structure) {
			new_mob.set_assigned_location(bases[base_index].get_random_defend_point());
		}
		else {
			new_mob.set_assigned_location(new_mob.get_home_location());
		}
	}
	mobH->addMob(new_mob);	
	Mob* mob = &mobH->getMob(*unit);
	if (!is_worker && !structure) {
		Directive atk_mv_to_defense(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, unit_type, sc2::ABILITY_ID::ATTACK_ATTACK, mob->get_assigned_location(), 2.0F);
		atk_mv_to_defense.executeForMob(this, mob);
	}
}

void BotAgent::OnBuildingConstructionComplete(const sc2::Unit* unit) {
	Mob* mob = &mobH->getMob(*unit);
	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_NEXUS ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
		unit_type == sc2::UNIT_TYPEID::ZERG_HATCHERY) {
		int base_index = get_index_of_closest_base(unit->pos);
		std::cout << "expansion " << base_index << " has been activated." << std::endl;
		bases[base_index].set_active();
	}
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_BUNKER ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_MISSILETURRET ||
		unit_type == sc2::UNIT_TYPEID::ZERG_SPINECRAWLER ||
		unit_type == sc2::UNIT_TYPEID::ZERG_SPORECRAWLER) {
		mob->set_flag(FLAGS::IS_DEFENSE);
	}
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERY ||
		unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR ||
		unit_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_REFINERYRICH ||
		unit_type == sc2::UNIT_TYPEID::ZERG_EXTRACTORRICH) {

		// assign 3 workers to harvest this
		for (int i = 0; i < 3; i++)
			AssignNearbyWorkerToGasStructure(*unit);
	}
}

void BotAgent::OnUnitDamaged(const sc2::Unit* unit, float health, float shields) {
	const sc2::ObservationInterface* observation = Observation();
	// make Stalkers Blink away if low health
	if (unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_STALKER) {
		if (have_upgrade(sc2::UPGRADE_ID::BLINKTECH)) {
			if (unit->health / unit->health_max < .5f) {
				// check if Blink is on cooldown
				if (can_unit_use_ability(*unit, sc2::ABILITY_ID::EFFECT_BLINK)) {
					Actions()->UnitCommand(unit, sc2::ABILITY_ID::EFFECT_BLINK, bases[0].get_townhall());
				}
			}
		}
	}
}

void BotAgent::OnUnitIdle(const sc2::Unit* unit) {
	Mob* mob = &mobH->getMob(*unit);
	mobH->set_mob_idle(mob, true);
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

void BotAgent::initLocations(int map_index, int p_id) {
	const sc2::ObservationInterface* observation = Observation();
	switch (map_index) {
	case 1:
		switch (p_id) {
		case 1:
		{
			Base main_base(observation->GetStartLocation());
			Base exp_1(66.5, 161.5);
			bases.push_back(exp_1);

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
			break;
		}
		break;
		}
	case 2:
		switch (p_id) {
		case 1:
		{
			enemy_location = sc2::Point2D(114.5, 25.5);
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
			break;
		}
		case 2:
		{
			enemy_location = sc2::Point2D(29.5, 134.5);
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
			break;
		}
		break;
		}
	case 3:
		switch (p_id) {
		case 1:
		{
			proxy_location = sc2::Point2D(28.0, 56.0);
			enemy_location = sc2::Point2D(62.5, 28.5);

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

			break;
		}
		case 2:
		{
			proxy_location = sc2::Point2D(172.0, 112.0);
			enemy_location = sc2::Point2D(137.5, 139.5);
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

			break;
		}
		break;
		}
	}
}