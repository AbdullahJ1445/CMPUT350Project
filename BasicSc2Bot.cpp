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

Directive* BasicSc2Bot::getLastStoredDirective()
{
	return directive_storage.back().get();
}


bool BasicSc2Bot::have_upgrade(const sc2::UpgradeID upgrade_) {
	// return true if the bot has fully researched the specified upgrade

	const sc2::ObservationInterface* observation = Observation();
	const std::vector<sc2::UpgradeID> upgrades = observation->GetUpgrades();
	return (std::find(upgrades.begin(), upgrades.end(), upgrade_) != upgrades.end());
}


bool BasicSc2Bot::can_unit_use_ability(const sc2::Unit& unit, const sc2::ABILITY_ID ability_) {
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



bool BasicSc2Bot::is_structure(const sc2::Unit* unit) {
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

bool BasicSc2Bot::is_mineral_patch(const sc2::Unit* unit_) {
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

bool BasicSc2Bot::is_geyser(const sc2::Unit* unit_) {
	// check whether a given unit is a geyser

	sc2::UNIT_TYPEID type_ = unit_->unit_type;
	return (type_ == sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER ||
		type_ == sc2::UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER);
}


std::vector<sc2::Attribute> BasicSc2Bot::get_attributes(const sc2::Unit* unit) {
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

	if (is_structure(unit)) {
		// structures locations should be high priority targets
		value *= 10;
	}

	return value;
}

void BasicSc2Bot::initVariables() {
	const sc2::ObservationInterface* observation = Observation();
	map_name = observation->GetGameInfo().map_name;
	if (map_name.find("Proxima") != std::string::npos) {
		std::cout << "!3!";
	}
	if (map_name.find("Vestige") != std::string::npos) {
		std::cout << "!2!";
	}
	if (map_name.find("Cactus") != std::string::npos) {
		std::cout << "!1!";
	}
	std::cout << "map_name: " << map_name;
	if (map_name == "Proxima Station LE" || map_name == "Proxima Station LE (Void)" || map_name == "(2)Proxima Station LE" || map_name == "(2)Proxima Station LE (Void)")
		map_index = 3; else
		if (map_name == "Bel'Shir Vestige LE" || map_name == "Bel'Shir Vestige LE (Void)" || map_name == "(2)Bel'Shir Vestige LE" || map_name == "(2)Bel'Shir Vestige LE (Void)")
			map_index = 2; else
			if (map_name == "Cactus Valley LE" || map_name == "Cactus Valley LE (Void)" || map_name == "(4)Cactus Valley LE" || map_name == "(4)Cactus Valley LE (Void)")
				map_index = 1; else
				map_index = 0;

	player_start_id = locH->getPlayerIDForMap(map_index, observation->GetStartLocation());
	locH->initLocations(map_index, player_start_id);

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
	std::cout << "Build Area 0: " << locH->bases[0].get_build_area(0).x << "," << locH->bases[0].get_build_area(0).y << std::endl;
	std::cout << "Proxy Location: " << proxy_location.x << "," << proxy_location.y << std::endl;
}

void::BasicSc2Bot::OnStep_100(const sc2::ObservationInterface* obs) {
	// occurs every 100 steps
	if (locH->chunksInitialized()) {
		locH->calculateHighestThreatForChunks();
	}
}

void::BasicSc2Bot::OnStep_1000(const sc2::ObservationInterface* obs) {
	// occurs every 1000 steps
	sc2::Point2D pathable_threat_spot = locH->getHighestThreatLocation(true, false);
	sc2::Point2D pathable_away_threat_spot = locH->getHighestThreatLocation();
	MapChunk* pathable_threat_chunk = locH->getChunkByCoords(std::pair<float, float>(pathable_threat_spot.x, pathable_threat_spot.y));
	MapChunk* pathable_away_threat_chunk = locH->getChunkByCoords(std::pair<float, float>(pathable_away_threat_spot.x, pathable_away_threat_spot.y));
	if (pathable_threat_spot == NO_POINT_FOUND) {
		//std::cout << "no pathable high threat location found" << std::endl;
	}
	else {
		std::cout << "highest threat at " << pathable_threat_spot.x << "," << pathable_threat_spot.y << " = " << pathable_threat_chunk->getThreat() << std::endl;;
	}
	/*
	if (pathable_away_threat_spot == NO_POINT_FOUND) {
		//std::cout << "no pathable high threat location found" << std::endl;
	}
	else {
		std::cout << "highest pathable threat away from start at " << pathable_away_threat_spot.x << "," << pathable_away_threat_spot.y << " = " << pathable_away_threat_chunk->getThreat() << std::endl;;
	}
	*/

}

void BasicSc2Bot::OnStep() {
	const sc2::ObservationInterface* observation = Observation();
	int gameloop = observation->GetGameLoop();
	if (gameloop % 100 == 0) {
		OnStep_100(observation);
	}
	if (gameloop % 1000 == 0) {
		OnStep_1000(observation);
	}

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
				for (int i = 0; i < 4; i++) {
					if (chunks[i]->isPathable()) {
						if (!found_pathable) {
							chunks[i]->increaseThreat(this, *it, 1.0);
							found_pathable = true;
						}
						else {
							chunks[i]->increaseThreat(this, *it, NEARBY_THREAT_MODIFIER);
						}
					}
				}
			}
			++it;
		}
	}
	

	//sc2::Point2D high_threat = locH->getHighestThreatLocation();
	//if (high_threat != INVALID_POINT) {
	//	std::cout << "HIGH THREAT LOCATION: (" << high_threat.x << ", " << high_threat.y << ")" << std::endl;
	//}

	/*
	std::unordered_set<Mob*> busy_mobs = mobH->get_busy_mobs();
	if (!busy_mobs.empty()) {
		for (auto it = busy_mobs.begin(); it != busy_mobs.end(); ++it) {
			Mob* m = *it;
			Directive* dir = m->getCurrentDirective();

			if (dir) {
				// if a busy mob has changed orders
				if (dir->getAbilityID() != (m->unit.orders).front().ability_id) {

					// free up the directive it was previously assigned
					dir->unassignMob(m);
				}
			}
		}
	}
	*/
	
	std::unordered_set<Mob*> idle_mobs = mobH->get_idle_mobs();
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
}

void BasicSc2Bot::OnUnitCreated(const sc2::Unit* unit) {
	const sc2::ObservationInterface* observation = Observation();
	
	// mob already exists
	if (mobH->mob_exists(*unit))
		return;

	sc2::UNIT_TYPEID unit_type = unit->unit_type;
	MOB mob_type; // which category of mob to create

	// determine if unit is a structure
	bool structure = is_structure(unit);
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

				std::unordered_set<Mob*> gas_builders = mobH->filter_by_flag(mobH->get_mobs(), FLAGS::BUILDING_GAS);
				Mob* gas_builder = Directive::get_closest_to_location(gas_builders, unit->pos);
				gas_builder->remove_flag(FLAGS::BUILDING_GAS);
				Actions()->UnitCommand(&gas_builder->unit, sc2::ABILITY_ID::STOP);
			}
		}
	}
	Mob new_mob(*unit, mob_type);

	if (new_mob.unit.is_flying) {
		new_mob.set_flag(FLAGS::IS_FLYING);
	}

	new_mob.set_home_location(locH->bases[base_index].get_townhall());
	if (is_worker) {
		new_mob.set_assigned_location(new_mob.get_home_location());
		Directive directive_get_minerals_near_birth(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION,
			new_mob.unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, locH->bases[base_index].get_townhall());
		new_mob.assignDefaultDirective(directive_get_minerals_near_birth);
	}
	else {
		if (!structure) {
			new_mob.set_assigned_location(Directive::uniform_random_point_in_circle(new_mob.get_home_location(), 2.5F));
		}
		else {
			new_mob.set_assigned_location(new_mob.get_home_location());
		}
	}
	mobH->addMob(new_mob);	
	Mob* mob = &mobH->getMob(*unit);
	/*
	if (!is_worker && !structure) {
		Directive atk_mv_to_defense(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, unit_type, sc2::ABILITY_ID::ATTACK_ATTACK, mob->get_assigned_location(), 2.0F);
		storeDirective(atk_mv_to_defense);
		Directive* dir = getLastStoredDirective();
		dir->executeForMob(this, mob);
	} */
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
		locH->bases[base_index].set_active();
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

	// set all buildings rally point (except townhalls)
	if (!is_townhall) {
		int nearest_base_idx = locH->getIndexOfClosestBase(unit->pos);
		sc2::Point2D rally_point = locH->bases[nearest_base_idx].get_rally_point();
		Actions()->UnitCommand(unit, sc2::ABILITY_ID::SMART, rally_point);
	}
}

void BasicSc2Bot::OnUnitDamaged(const sc2::Unit* unit, float health, float shields) {
	const sc2::ObservationInterface* observation = Observation();
	// make Stalkers Blink away if low health
	if (unit->unit_type == sc2::UNIT_TYPEID::PROTOSS_STALKER) {
		if (have_upgrade(sc2::UPGRADE_ID::BLINKTECH)) {
			if (unit->health / unit->health_max < .3f) {
				// check if Blink is on cooldown
				if (can_unit_use_ability(*unit, sc2::ABILITY_ID::EFFECT_BLINK)) {
					std::cout << "(blink)";
					Actions()->UnitCommand(unit, sc2::ABILITY_ID::EFFECT_BLINK, locH->bases[0].get_townhall());
				}
			}
		}
	}
}

void BasicSc2Bot::OnUnitIdle(const sc2::Unit* unit) {
	Mob* mob = &mobH->getMob(*unit);
	mobH->set_mob_idle(mob, true);
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
}