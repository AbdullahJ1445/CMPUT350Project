#include "BasicSc2Bot.h"
#include "Directive.h"
#include "Mob.h"
#include "LocationHandler.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2api/sc2_client.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#define MINERAL_VALUE 2
#define GAS_VALUE 3
#define FOOD_VALUE 100

// forward declarations
class TriggerCondition;
class Mob;
class MobHandler;


BasicSc2Bot::BasicSc2Bot()
{
	mobH = nullptr;
	locH = nullptr;
	proxy_worker = nullptr;
	current_strategy = nullptr;
	player_start_id = -1;
	enemy_start_id = -1;
	map_name = "";
	enemy_race = sc2::Race::Random;
	map_index = 0;
	timer_1 = -1;
	timer_2 = -1;
	timer_3 = -1;
}

void BasicSc2Bot::setCurrentStrategy(Strategy* strategy_) {
	// set the current strategy
	storeStrategy(*strategy_);
}

void BasicSc2Bot::addStrat(Precept precept_) {
	assert(precept_.hasDirective());
	assert(precept_.hasTrigger());
	precepts_onstep.push_back(precept_);
}

bool BasicSc2Bot::AssignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure) {
	// get a nearby worker unit that is not currently assigned to gas
	// and assign it to harvest gas

	bool found_viable_unit = false;
	
	// get a filtered set of workers that are currently assigned to minerals
	std::unordered_set<FLAGS> flags{ FLAGS::IS_WORKER, FLAGS::IS_MINERAL_GATHERER };
	std::unordered_set<Mob*> worker_miners = mobH->filterByFlags(mobH->getMobs(), flags);

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
		target->removeFlag(FLAGS::IS_MINERAL_GATHERER);
		target->setFlag(FLAGS::IS_GAS_GATHERER);

		// make the unit continue to mine gas after being idle
		Directive directive_get_gas(Directive::DEFAULT_DIRECTIVE, Directive::GET_GAS_NEAR_LOCATION, target->unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, gas_structure.pos);
		target->assignDefaultDirective(directive_get_gas);
		Actions()->UnitCommand(&(target->unit), sc2::ABILITY_ID::HARVEST_GATHER, &gas_structure);


		return true;
	}
	return false;
}

void BasicSc2Bot::storeDirective(Directive directive_)
{
	if (directive_by_id[directive_.getID()])
		return;

	directive_storage.emplace_back(std::make_unique<Directive>(directive_));
	Directive* created_dir = directive_storage.back().get();
	stored_directives.insert(created_dir);
	directive_by_id[directive_.getID()] = created_dir;
}

void BasicSc2Bot::storeStrategy(Strategy strategy_)
{
	// only one strategy can be stored
	if (strategy_storage.empty()) {
		strategy_storage.emplace_back(std::make_unique<Strategy>(strategy_));
		current_strategy = strategy_storage.back().get();
	}
}

void BasicSc2Bot::setTimer1(int steps_) {
	// sets timer1 only if its value is not already set
	if (timer_1 == -1) {
		const sc2::ObservationInterface* obs = Observation();
		auto current_steps = obs->GetGameLoop();
		timer_1 = current_steps + steps_;
	}
}

void BasicSc2Bot::setTimer2(int steps_) {
	// sets timer2 only if its value is not already set
	if (timer_2 == -1) {
		const sc2::ObservationInterface* obs = Observation();
		auto current_steps = obs->GetGameLoop();
		timer_2 = current_steps + steps_;
	}
}

void BasicSc2Bot::setTimer3(int steps_) {
	// sets timer3 only if its value is not already set
	if (timer_3 == -1) {
		const sc2::ObservationInterface* obs = Observation();
		auto current_steps = obs->GetGameLoop();
		timer_3 = current_steps + steps_;
	}
}

int BasicSc2Bot::getStepsPastTimer1()
{
	// get the amount of timesteps past timer_1

	if (timer_1 != -1) {
		const sc2::ObservationInterface* obs = Observation();
		auto current_steps = obs->GetGameLoop();
		return current_steps - timer_1;
	}
	return -1;
}

int BasicSc2Bot::getStepsPastTimer2() {
	// get the amount of timesteps past timer_2

	if (timer_2 != -1) {
		const sc2::ObservationInterface* obs = Observation();
		auto current_steps = obs->GetGameLoop();
		return current_steps - timer_2;
	}
	return -1;
}

int BasicSc2Bot::getStepsPastTimer3() {
	// get the amount of timesteps past timer_1

	if (timer_3 != -1) {
		const sc2::ObservationInterface* obs = Observation();
		auto current_steps = obs->GetGameLoop();
		return current_steps - timer_3;
	}
	return -1;
}

int BasicSc2Bot::getTimer1Value() {
	// returns -1 if timer was reset
	return timer_1;
}

int BasicSc2Bot::getTimer2Value() {
	// returns -1 if timer was reset
	return timer_2;
}

int BasicSc2Bot::getTimer3Value() {
	// returns -1 if timer was reset
	return timer_3;
}

void BasicSc2Bot::resetTimer1() {
	// resets timer_1 so it can be given a new value
	timer_1 = -1;
}

void BasicSc2Bot::resetTimer2() {
	// resets timer_2 so it can be given a new value
	timer_2 = -1;
}

void BasicSc2Bot::resetTimer3() {
	// resets timer_3 so it can be given a new value
	timer_3 = -1;
}

Directive* BasicSc2Bot::getLastStoredDirective()
{
	return directive_storage.back().get();
}


bool BasicSc2Bot::haveUpgrade(const sc2::UpgradeID upgrade_) {
	// return true if the bot has fully researched the specified upgrade

	const sc2::ObservationInterface* observation = Observation();
	const std::vector<sc2::UpgradeID> upgrades = observation->GetUpgrades();
	return (std::find(upgrades.begin(), upgrades.end(), upgrade_) != upgrades.end());
}


bool BasicSc2Bot::canUnitUseAbility(const sc2::Unit& unit, const sc2::ABILITY_ID ability_) {
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



bool BasicSc2Bot::isStructure(const sc2::Unit* unit) {
	// check if unit is a structure
	return (std::find(data_buildings.begin(), data_buildings.end(), unit->unit_type) != data_buildings.end());
}

void BasicSc2Bot::storeUnitType(std::string identifier_, sc2::UNIT_TYPEID unit_type_)
{
	// store a special unit, used for certain functions
	// e.g. "_CHRONOBOOST_TARGET" : the unit type of a building that chronoboost will exclusively target
	special_units[identifier_] = unit_type_;
}

void BasicSc2Bot::storeLocation(std::string identifier_, sc2::Point2D location_) {
	// store a special location
	// can be used in strategies to assign specific locations to later reference
	special_locations[identifier_] = location_;
}

bool BasicSc2Bot::isMineralPatch(const sc2::Unit* unit_) {
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

sc2::UNIT_TYPEID BasicSc2Bot::getUnitType(std::string identifier_)
{
	return special_units[identifier_];
	
}

sc2::Race BasicSc2Bot::getEnemyRace() {
	return enemy_race;
}

std::unordered_set<const sc2::Unit*> BasicSc2Bot::getEnemyUnits()
{
	return enemy_units;
}

sc2::Point2D BasicSc2Bot::getStoredLocation(std::string identifier_)
{
	return special_locations[identifier_];
}

int BasicSc2Bot::getMapIndex()
{
	// 1: cactus
	// 2: belshir
	// 3: proxima
	return map_index;
}

bool BasicSc2Bot::isGeyser(const sc2::Unit* unit_) {
	// check whether a given unit is a geyser

	sc2::UNIT_TYPEID type_ = unit_->unit_type;
	return (type_ == sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
}


std::vector<sc2::Attribute> BasicSc2Bot::getAttributes(const sc2::Unit* unit) {
	// get attributes for a unit

	return getUnitTypeData(unit).attributes;
}

sc2::UnitTypeData BasicSc2Bot::getUnitTypeData(const sc2::Unit* unit) {
	// get UnitTypeData for a unit

	return Observation()->GetUnitTypeData()[unit->unit_type];
}

bool BasicSc2Bot::addEnemyUnit(const sc2::Unit* unit) {
	if (enemy_unit_by_tag[unit->tag] != nullptr) {
		return false;
	}
	enemy_units.insert(unit);
	enemy_unit_by_tag[unit->tag] = unit;
	return true;
}

bool BasicSc2Bot::flushOrders()
{
	bool any_flushed = false;
	std::unordered_set<Mob*> busy_mobs = mobH->getBusyMobs();
	for (auto m : busy_mobs) {
		if (!m->hasCurrentDirective()) {
			mobH->setMobIdle(m);
			any_flushed = true;
			continue;
		}
		Directive* current_dir = m->getCurrentDirective();
		sc2::ABILITY_ID current_ability = current_dir->getAbilityID();
		auto orders = m->unit.orders;
		if (orders.empty()) {
			current_dir->unassignMob(m);
			mobH->setMobIdle(m);
			any_flushed = true;
			Actions()->UnitCommand(&m->unit, sc2::ABILITY_ID::STOP);
			continue;
		}
		auto order = orders.front();
		if (order.ability_id != current_ability) {
			current_dir->unassignMob(m);
			mobH->setMobIdle(m);
			any_flushed = true;
			continue;
		}
	}
	return any_flushed;
}

int BasicSc2Bot::getMineralCost(const sc2::Unit* unit) {
	return mineral_cost[(int)unit->unit_type];
}

int BasicSc2Bot::getGasCost(const sc2::Unit* unit) {
	return gas_cost[(int)unit->unit_type];
}

int BasicSc2Bot::getFoodCost(const sc2::Unit* unit) {
	return food_cost[(int)unit->unit_type];
}

float BasicSc2Bot::getValue(const sc2::Unit* unit) {
	int mineral_cost = getMineralCost(unit);
	int gas_cost = getGasCost(unit);
	int food_cost = getFoodCost(unit);


	float value = mineral_cost * MINERAL_VALUE + gas_cost * GAS_VALUE + food_cost * FOOD_VALUE;

	if (isStructure(unit)) {
		// structures locations should be high priority targets
		value *= 10;
	}

	return value;
}

void BasicSc2Bot::initVariables() {
	const sc2::ObservationInterface* observation = Observation();
	map_name = observation->GetGameInfo().map_name;

	std::cout << "map_name: " << map_name;
	if (map_name.find("Proxima") != std::string::npos)
		map_index = 3; else
		if (map_name.find("Vestige") != std::string::npos)
			map_index = 2; else
			if (map_name.find("Cactus") != std::string::npos)
				map_index = 1; else
				map_index = 0;

	player_start_id = locH->getPlayerIDForMap(map_index, observation->GetStartLocation());
	locH->initLocations(map_index, player_start_id);
	enemy_race = sc2::Race::Random;

	std::cout << "Map Index " << map_index << std::endl;
	std::cout << "Map Name: " << map_name << std::endl;
	std::cout << "Player Start ID: " << player_start_id << std::endl;
}

void BasicSc2Bot::initStartingUnits() {
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
			Directive directive_get_minerals_near_Base(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION, u_type, sc2::ABILITY_ID::HARVEST_GATHER, ASSIGNED_LOCATION);
			storeDirective(directive_get_minerals_near_Base);
			Directive* dir = getLastStoredDirective();
			worker.assignDefaultDirective(*dir);
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

void BasicSc2Bot::OnGameStart() {
	Strategy* strategy = new Strategy(this);
	setCurrentStrategy(strategy);

	mobH = new MobHandler(this); // initialize mob handler 
	locH = new LocationHandler(this);
	BasicSc2Bot::initVariables(); // initialize this first
	BasicSc2Bot::initStartingUnits();
	sc2::Point2D start_location = locH->getStartLocation();
	sc2::Point2D proxy_location = locH->getProxyLocation();
	current_strategy->loadStrategies();

	const sc2::ObservationInterface* obs = Observation();
	auto utd_fulldata = obs->GetUnitTypeData();
	auto atd_fulldata = obs->GetAbilityData();

	// populate data_buildings - vector of all unit types which are buildings
	std::cout << "populating data_buildings";
	int i = 0;
	for (auto utd : utd_fulldata) {
		if (i % 500 == 0) {
			std::cout << ".";
		}
		if (utd.mineral_cost > 0 && std::find(utd.attributes.begin(), utd.attributes.end(), sc2::Attribute::Structure) != utd.attributes.end()) {
			//std::cout << utd.name << " id: " << utd.unit_type_id << "  alias: " << utd.unit_alias << std::endl;
			data_buildings.push_back(utd.unit_type_id);
		}
		if (utd.mineral_cost > 0 || utd.food_required > 0 || utd.vespene_cost > 0) {
			auto a = utd.unit_type_id;
			int c = a;
			mineral_cost[(int)utd.unit_type_id] = utd.mineral_cost;
			gas_cost[(int)utd.unit_type_id] = utd.vespene_cost;
			food_cost[(int)utd.unit_type_id] = utd.food_required;
		}
		i++;
	}
	std::cout << " " << data_buildings.size() << " building types." << std::endl;

	std::cout << "Start Location: " << start_location.x << "," << start_location.y << std::endl;
	std::cout << "Build Area 0: " << locH->bases[0].getBuildArea(0).x << "," << locH->bases[0].getBuildArea(0).y << std::endl;
	std::cout << "Proxy Location: " << proxy_location.x << "," << proxy_location.y << std::endl;
}

void::BasicSc2Bot::OnStep_100(const sc2::ObservationInterface* obs) {
	// occurs every 100 steps
	if (locH->chunksInitialized()) {
		locH->calculateHighestThreatForChunks();
	}
	flushOrders();
}

void::BasicSc2Bot::OnStep_1000(const sc2::ObservationInterface* obs) {
	// occurs every 1000 steps
	static MapChunk* prev_threat_chunk = nullptr;
	static double prev_threat_amount = 0;
	sc2::Point2D pathable_threat_spot = locH->getHighestThreatLocation(true, false);
	MapChunk* pathable_threat_chunk = locH->getChunkByCoords(std::pair<float, float>(pathable_threat_spot.x, pathable_threat_spot.y));
	double threat_amount = 0;
	
	if (pathable_threat_spot != NO_POINT_FOUND) {
		threat_amount = pathable_threat_chunk->getThreat();
	}
	if (pathable_threat_chunk != prev_threat_chunk || threat_amount != prev_threat_amount) {
		if (pathable_threat_spot != NO_POINT_FOUND) {
			std::cout << "[" << obs->GetGameLoop() << "] highest threat at " << pathable_threat_spot.x << ", " << pathable_threat_spot.y << " = " << pathable_threat_chunk->getThreat() << std::endl;;
		}
		else {
			std::cout << "[" << obs->GetGameLoop() << "] no threats found." << std::endl;
		}
	}
	prev_threat_chunk = pathable_threat_chunk;
	prev_threat_amount = threat_amount;
}

void BasicSc2Bot::OnStep() {
	const sc2::ObservationInterface* observation = Observation();
	int gameloop = observation->GetGameLoop();

	// update visibility data for chunks
	locH->scanChunks(observation);
	if (!enemy_units.empty()) {
		for (auto it = enemy_units.begin(); it != enemy_units.end(); ) {
			auto next = std::next(it);
			if (!(*it)->is_alive) {
				// we do not need to keep dead enemy units in set
				enemy_units.erase(*it);
				it = next;
				continue;
			}
			// if unit is currently visible to you
			if ((*it)->last_seen_game_loop == gameloop) {
				std::vector<MapChunk*> chunks = locH->getLocalChunks((*it)->pos);

				// only increase threat for pathable chunks
				// the closest chunk should increase by a scale of 1.0, while other nearby chunks
				// increase by a lesser amount adjusted by NEARBY_THREAT_MODIFIER
				bool found_pathable = false; //whether we have found a pathable chunk near the unit
				for (auto chunk : chunks) {
					if (chunk->isPathable()) {
						if (!found_pathable) {
							chunk->increaseThreat(this, *it, 1.0);
							found_pathable = true;
						}
						else {
							chunk->increaseThreat(this, *it, NEARBY_THREAT_MODIFIER);
						}
					}
				}
			}
			++it;
		}
	}
	
	std::unordered_set<Mob*> busy_mobset = mobH->getBusyMobs();
	if (!busy_mobset.empty()) {
		for (auto it = busy_mobset.begin(); it != busy_mobset.end(); ) {
			auto next = std::next(it);
			Mob* m = *it;
			auto orders = m->unit.orders;
			if (m->hasCurrentDirective()) {
				Directive* dir = m->getCurrentDirective();
				if (dir) {
					// if a busy mob has changed orders
					
					if (!orders.empty()) {
						auto order = orders.front();
						if (dir->getAbilityID() != order.ability_id) {

							// free up the directive it was previously assigned
							dir->unassignMob(m);
						}
					}
				}
			}
			if (orders.empty()) {
				mobH->setMobBusy(m, false);
				Actions()->UnitCommand(&m->unit, sc2::ABILITY_ID::STOP);
			}
			it = next;
		}
	}
	
	
	std::unordered_set<Mob*> idle_mobs = mobH->getIdleMobs();
	if (!idle_mobs.empty()) {
		for (auto it = idle_mobs.begin(); it != idle_mobs.end(); ) {
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

	for (Precept s : precepts_onstep) {
		if (s.checkTriggerConditions()) {
			s.execute();
		}
	}

	if (gameloop % 100 == 0) {
		OnStep_100(observation);
	}
	if (gameloop % 1000 == 0) {
		OnStep_1000(observation);
	}
}

void BasicSc2Bot::OnUnitCreated(const sc2::Unit* unit) {
	const sc2::ObservationInterface* observation = Observation();
	
	// mob already exists
	if (mobH->mobExists(*unit))
		return;

	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	MOB mob_type; // which category of mob to create

	// determine if unit is a structure
	bool structure = isStructure(unit);
	bool is_worker = false;
	int base_index = locH->getIndexOfClosestBase(unit->pos);

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

				std::unordered_set<Mob*> gas_builders = mobH->filterByFlag(mobH->getMobs(), FLAGS::BUILDING_GAS);
				Mob* gas_builder = Directive::getClosestToLocation(gas_builders, unit->pos);
				gas_builder->removeFlag(FLAGS::BUILDING_GAS);
				Actions()->UnitCommand(&gas_builder->unit, sc2::ABILITY_ID::STOP);
			}
		}
	}
	Mob new_mob(*unit, mob_type);

	if (new_mob.unit.is_flying) {
		new_mob.setFlag(FLAGS::IS_FLYING);
	}
	else {
		new_mob.setFlag(FLAGS::GROUND);
	}

	if (mob_type == MOB::MOB_ARMY) {
		// todo: implement assigning flags for other races and units
		if (unit_type == sc2::UNIT_TYPEID::PROTOSS_STALKER ||
			unit_type == sc2::UNIT_TYPEID::PROTOSS_IMMORTAL) {
			new_mob.setFlag(FLAGS::SHORT_RANGE);
		}
	}

	new_mob.setHomeLocation(locH->bases[base_index].getTownhall());
	if (is_worker) {
		new_mob.setAssignedLocation(new_mob.getHomeLocation());
		Directive directive_get_minerals_near_birth(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION,
			new_mob.unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, ASSIGNED_LOCATION);
		storeDirective(directive_get_minerals_near_birth);
		Directive* dir = getLastStoredDirective();
		new_mob.assignDefaultDirective(*dir);
	}
	else {
		if (!structure) {
			new_mob.setAssignedLocation(locH->bases[locH->getIndexOfClosestBase(unit->pos)].getRallyPoint());
			//new_mob.setAssignedLocation(Directive::uniform_random_point_in_circle(new_mob.getHomeLocation(), 2.5F));
		}
		else {
			new_mob.setAssignedLocation(new_mob.getHomeLocation());
		}
	}
	mobH->addMob(new_mob);	
	Mob* mob = &mobH->getMob(*unit);
}

void BasicSc2Bot::OnBuildingConstructionComplete(const sc2::Unit* unit) {
	Mob* mob = &mobH->getMob(*unit);
	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	bool is_townhall = false;
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_NEXUS ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER ||
		unit_type == sc2::UNIT_TYPEID::ZERG_HATCHERY) {
		int base_index = locH->getIndexOfClosestBase(unit->pos);
		is_townhall = true;
		std::cout << "expansion " << base_index << " has been activated." << std::endl;
		locH->bases[base_index].setActive();
	}
	if (unit_type == sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_BUNKER ||
		unit_type == sc2::UNIT_TYPEID::TERRAN_MISSILETURRET ||
		unit_type == sc2::UNIT_TYPEID::ZERG_SPINECRAWLER ||
		unit_type == sc2::UNIT_TYPEID::ZERG_SPORECRAWLER) {
		mob->setFlag(FLAGS::IS_DEFENSE);
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

	// set all buildings rally point (except townhalls)
	if (!is_townhall) {
		int nearest_base_idx = locH->getIndexOfClosestBase(unit->pos);
		sc2::Point2D rally_point = locH->bases[nearest_base_idx].getRallyPoint();
		Actions()->UnitCommand(unit, sc2::ABILITY_ID::SMART, rally_point);
	}
}

void BasicSc2Bot::OnUnitDamaged(const sc2::Unit* unit, float health, float shields) {
	const sc2::ObservationInterface* observation = Observation();
	// make Stalkers Blink away if low health
	if (unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_STALKER) {
		if (haveUpgrade(sc2::UPGRADE_ID::BLINKTECH)) {
			if (unit->health / unit->health_max < .3f) {
				// check if Blink is on cooldown
				if (canUnitUseAbility(*unit, sc2::ABILITY_ID::EFFECT_BLINK)) {
					//std::cout << "(blink)";
					Actions()->UnitCommand(unit, sc2::ABILITY_ID::EFFECT_BLINK, locH->bases[0].getTownhall());
				}
			}
		}
	}

	auto buffs = unit->buffs;
	bool guardian_shield = false;
	if (!buffs.empty()) {
		guardian_shield = std::find(buffs.begin(), buffs.end(), sc2::BUFF_ID::GUARDIANSHIELD) != buffs.end();
	}

	// todo : check if player race = protoss
	// engage sentry guardian shield when a nearby unit takes damage
	if (!guardian_shield) {
		std::unordered_set<Mob*> mobs = mobH->getMobs();
		mobs = Directive::filterNearLocation(mobs, unit->pos, 4.5F);
		std::unordered_set<Mob*> sentries;
		std::copy_if(mobs.begin(), mobs.end(), std::inserter(sentries, sentries.begin()),
			[this](Mob* m) { return (m->unit.unit_type == sc2::UNIT_TYPEID::PROTOSS_SENTRY); });
		if (!sentries.empty()) {
			std::unordered_set<Mob*> sentries_filter;
			std::copy_if(sentries.begin(), sentries.end(), std::inserter(sentries_filter, sentries_filter.begin()),
				[this](Mob* m) { return canUnitUseAbility(m->unit, sc2::ABILITY_ID::EFFECT_GUARDIANSHIELD); });
			if (!sentries_filter.empty()) {
				Mob* sentry = *sentries_filter.begin();
				Actions()->UnitCommand(&sentry->unit, sc2::ABILITY_ID::EFFECT_GUARDIANSHIELD);
			}
		}
	}

	// increase threat slightly on damage taken for situations where an enemy out of sight is dealing damage to us
	bool found_pathable = false; //whether we have found a pathable chunk near the unit taking damage
	std::vector<MapChunk*> chunks = locH->getLocalChunks(unit->pos);
	for (auto chunk : chunks) {
		if (chunk->isPathable()) {
			if (!found_pathable) {
				chunk->increaseThreat(this, unit, 0.02);
				found_pathable = true;
			}
			else {
				chunk->increaseThreat(this, unit, 0.01);
			}
		}
	}

}

void BasicSc2Bot::OnUnitIdle(const sc2::Unit* unit) {
	Mob* mob = &mobH->getMob(*unit);
	mobH->setMobIdle(mob, true);
	if (mob->hasCurrentDirective()) {
		Directive* prev_dir = mob->getCurrentDirective();
		size_t id = prev_dir->getID();
		prev_dir->unassignMob(mob);
		mob->unassignDirective();
	}
	

}

void BasicSc2Bot::OnUnitDestroyed(const sc2::Unit* unit) {
	if (unit->alliance == sc2::Unit::Alliance::Self) {
		
		Mob* mob = &mobH->getMob(*unit);

		// if mob had directive assigned, free it up
		Directive* dir = mob->getCurrentDirective();
		if (dir) {
			mob->unassignDirective();
			dir->unassignMob(mob);
		}
		mobH->mobDeath(mob);
	}
}

void BasicSc2Bot::OnUnitEnterVision(const sc2::Unit* unit) {
	addEnemyUnit(unit);
	if (enemy_race == sc2::Race::Random) {
		const sc2::ObservationInterface* obs = Observation();
		auto utd_vector = obs->GetUnitTypeData();
		auto utd = utd_vector[unit->unit_type];
		enemy_race = utd.race;
		if (enemy_race == sc2::Race::Protoss) {
			std::cout << "Enemy Race Detected: Protoss" << std::endl;
		}
		if (enemy_race == sc2::Race::Terran) {
			std::cout << "Enemy Race Detected: Terran" << std::endl;
		}
		if (enemy_race == sc2::Race::Zerg) {
			std::cout << "Enemy Race Detected: Zerg" << std::endl;
		}
	}
	/* needs debugging 
	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	if (unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANK || unit_type == sc2::UNIT_TYPEID::TERRAN_SIEGETANKSIEGED) {
		mobH->nearbyMobsWithFlagsAttackTarget(std::unordered_set<FLAGS>{FLAGS::SHORT_RANGE}, unit, 8.0F);
	} */
}