#include "Mob.h"

Mob::Mob(const sc2::Unit& unit_, MOB mobs_type) : unit(unit_) {
	initVars();

	if (mobs_type == MOB::MOB_WORKER) {
		// default all trained workers to be mineral gatherers for now
		// true flags
		flags.insert(FLAGS::IS_WORKER);
	}

	if (mobs_type == MOB::MOB_ARMY) {
		flags.insert(FLAGS::IS_ATTACKER);
	}

	if (mobs_type == MOB::MOB_TOWNHALL) {
		// true flags
		flags.insert(FLAGS::IS_STRUCTURE);
		flags.insert(FLAGS::IS_TOWNHALL);
		flags.insert(FLAGS::IS_SUPPLY);
		flags.insert(FLAGS::IS_CONSTRUCTING);
	}
	if (mobs_type == MOB::MOB_STRUCTURE) {
		// true flags
		flags.insert(FLAGS::IS_STRUCTURE);
		flags.insert(FLAGS::IS_CONSTRUCTING);
	}
}

void Mob::initVars() {
	// initialize all flags to false
	tag = unit.tag;
	cooldown = 0;
	has_default_directive = false;
	has_bundled_directive = false;
	has_current_directive = false;
	is_harvesting_gas = false;
	is_harvesting_minerals = false;
	birth_location = unit.pos;
	home_location = unit.pos;
	assigned_location = unit.pos;
	current_directive = nullptr;
	std::unordered_set<FLAGS> flags;
	std::unordered_set<Mob*> harvesters;
	gas_structure_harvested = nullptr;
	townhall_for_minerals = nullptr;
}

bool Mob::isIdle() {
	return ((unit.orders).size() == 0);
}

bool Mob::hasFlag(FLAGS flag) {
	return (flags.find(flag) != flags.end());
}

void Mob::assignDefaultDirective(BasicSc2Bot* agent, Directive* directive_) {
	if (has_default_directive) {
		delete default_directive;
	}
	directive_->setDefault(); // change directive type to default directive
	default_directive = agent->directive_by_id[directive_->getID()];
	has_default_directive = true;
}

void Mob::assignDirective(Directive* directive_) {
	// set the mob's current directive
	current_directive = directive_;
	has_current_directive = true;
}

void Mob::unassignDirective() {
	// unassign the mob's current directive
	current_directive = nullptr;
	has_current_directive = false;
}

bool Mob::hasDefaultDirective() {
	return has_default_directive;
}

bool Mob::hasBundledDirective() {
	// Return whether the mob has bundled directives in queue 
	return has_bundled_directive;
}

bool Mob::hasCurrentDirective() {
	return has_current_directive;
}


bool Mob::executeDefaultDirective(BasicSc2Bot* agent) {
	if (has_default_directive) {
		return default_directive->executeForMob(agent, this);
	}
	return false;
}

void Mob::disableDefaultDirective()
{
	has_default_directive = false;
}

void Mob::bundleDirectives(std::vector<Directive> dir_vec) {
	// bundle the orders to be executed after this unit is 
	// completed its current order
	if (hasBundledDirective())
		delete bundled_directive;

	Directive* direc = new Directive(dir_vec.front());
	dir_vec.erase(dir_vec.begin());

	if (dir_vec.size() > 0) {
		for (auto d : dir_vec) {
			direc->bundleDirective(d);
		}
	}
	has_bundled_directive = true;
	bundled_directive = direc;
}


void Mob::setFlag(FLAGS flag) {
	flags.insert(flag);
}

void Mob::removeFlag(FLAGS flag) {
	flags.erase(flag);
}

void Mob::giveCooldown(BasicSc2Bot* agent, int amt)
{
	// put this mob on cooldown
	cooldown = agent->Observation()->GetGameLoop() + amt;
}

Directive Mob::popBundledDirective() {
	// get the directive bundled on this mob and simultaneously remove it from this mob
	Directive bundled = *bundled_directive;
	delete bundled_directive;
	has_bundled_directive = false;
	return bundled;
}

bool Mob::isCarryingMinerals() {
	// return true if a unit is carrying minerals

	std::vector<sc2::BuffID> unit_buffs = unit.buffs;
	std::vector<sc2::BuffID> mineral_buffs{
		sc2::BUFF_ID::CARRYHIGHYIELDMINERALFIELDMINERALS,
		sc2::BUFF_ID::CARRYMINERALFIELDMINERALS
	};

	return std::find_first_of(unit_buffs.begin(), unit_buffs.end(), mineral_buffs.begin(), mineral_buffs.end()) != unit_buffs.end();
}

bool Mob::isCarryingGas() {
	// return true if a unit is carrying gas

	std::vector<sc2::BuffID> unit_buffs = unit.buffs;
	std::vector<sc2::BuffID> gas_buffs{
		sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGAS,
		sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASPROTOSS,
		sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASZERG
	};

	return std::find_first_of(unit_buffs.begin(), unit_buffs.end(), gas_buffs.begin(), gas_buffs.end()) != unit_buffs.end();
}

bool Mob::isOnCooldown(BasicSc2Bot* agent)
{
	// returns whether this unit is on cooldown (i.e. should not take commands)

	return (agent->Observation()->GetGameLoop() < cooldown);
}

void Mob::setHomeLocation(sc2::Point2D location) {
	home_location = location;
}

void Mob::setAssignedLocation(sc2::Point2D location) {
	assigned_location = location;
}

sc2::Point2D Mob::getBirthLocation() {
	return birth_location;
}

sc2::Point2D Mob::getHomeLocation() {
	return home_location;
}

sc2::Point2D Mob::getAssignedLocation() {
	return assigned_location;
}

std::unordered_set<FLAGS> Mob::getFlags() {
	return flags;
}

bool Mob::setCurrentDirective(Directive* directive_) {
	// Do not allow if the directive does not allow for multiple mobs and already has an associated mob
	
	if (!directive_->allowsMultiple() && directive_->hasAssignedMob()) {
		return false;
	}
	current_directive = directive_;
	directive_->assignMob(this);
	return true;
}

Directive* Mob::getCurrentDirective() {
	if (has_current_directive) {
		return current_directive;
	}
	else {
		return nullptr;
	}
}

void Mob::setHarvestingMinerals(Mob* townhall_) {
	assert(townhall_->hasFlag(FLAGS::IS_TOWNHALL));
	if (is_harvesting_minerals) {
		if (townhall_ == townhall_for_minerals) {
			return;
		}
		if (townhall_for_minerals != nullptr) {
			townhall_for_minerals->removeHarvester(this);
	}
		townhall_for_minerals = townhall_;
		townhall_->addHarvester(this);
		is_harvesting_minerals = true;
	}
}

void Mob::setHarvestingGas(Mob* gas_structure_) {
	assert(gas_structure_->hasFlag(FLAGS::IS_GAS_STRUCTURE));
	if (is_harvesting_gas) {
		if (gas_structure_ == gas_structure_harvested) {
			return;
		}
		if (gas_structure_harvested != nullptr) {
			gas_structure_harvested->removeHarvester(this);
		}
	}
	gas_structure_harvested = gas_structure_;
	gas_structure_->addHarvester(this);
	is_harvesting_gas = true;
}

Mob* Mob::getGasStructureHarvesting() {
	return gas_structure_harvested;
}

Mob* Mob::getTownhallForMinerals() {
	return townhall_for_minerals;
}

bool Mob::isHarvestingGas() {
	return is_harvesting_gas;
}

bool Mob::isHarvestingMinerals() {
	return is_harvesting_minerals;
}

void Mob::stopHarvestingGas() {
	// set this unit to stop harvesting a gas structure
	// does not change the default directive

	if (is_harvesting_gas) {
		removeFlag(FLAGS::IS_GAS_GATHERER);
		if (gas_structure_harvested != nullptr) {
			gas_structure_harvested->removeHarvester(this);
		}
		gas_structure_harvested = nullptr;
		is_harvesting_gas = false;
	}
}

void Mob::stopHarvestingMinerals() {
	// set this unit to stop harvesting minerals
	// does not change the default directive

	if (is_harvesting_minerals) {
		removeFlag(FLAGS::IS_MINERAL_GATHERER);
		if (townhall_for_minerals != nullptr) {
			townhall_for_minerals->removeHarvester(this);
		}
		townhall_for_minerals = nullptr;
		is_harvesting_minerals = false;
	}
}

void Mob::addHarvester(Mob* mob_) {
	assert(hasFlag(FLAGS::IS_GAS_STRUCTURE) || hasFlag(FLAGS::IS_TOWNHALL));
	assert(mob_->hasFlag(FLAGS::IS_WORKER));
	harvesters.insert(mob_);
}

void Mob::removeHarvester(Mob* mob_) {
	// remove harvester from this gas structure or townhall (for minerals)

	//assert(hasFlag(FLAGS::IS_GAS_STRUCTURE) || hasFlag(FLAGS::IS_TOWNHALL));

	if (!harvesters.empty()) {
		for (auto it = harvesters.begin(); it != harvesters.end(); ) {
			auto next = std::next(it);
			if ((*it) == mob_) {
				harvesters.erase(*it);
			}
			it = next;
		}
	}
}

std::unordered_set<Mob*> Mob::getHarvesters() {
	// get a set of all mobs harvesting this gas structure

	return harvesters;
}

int Mob::getHarvesterCount() {
	return harvesters.size();
}

bool Mob::harvestNearbyTownhall(BasicSc2Bot* agent)
{
	// assigns this worker to harvest minerals near a townhall

	if (isHarvestingGas()) {
		stopHarvestingGas();
	}

	if (isHarvestingMinerals()) {
		stopHarvestingMinerals();
	}

	auto townhalls = agent->mobH->filterByFlag(agent->mobH->getMobs(), FLAGS::IS_TOWNHALL);
	if (townhalls.empty()) {
		return false;
	}

	setFlag(FLAGS::IS_MINERAL_GATHERER);

	const sc2::Unit* mineral_target = agent->locH->getNearestMineralPatch(unit.pos);

	Mob* townhall = Directive::getClosestToLocation(townhalls, mineral_target->pos);

	setHarvestingMinerals(townhall);
	setAssignedLocation(townhall->unit.pos);
	Directive directive_get_minerals(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION, unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, unit.pos);
	agent->storeDirective(directive_get_minerals);
	Directive* dir = agent->getLastStoredDirective();
	if (hasDefaultDirective()) {
		if (default_directive != nullptr) {
			default_directive->unassignMob(this);
		}
	}
	assignDefaultDirective(agent, dir);
	
	if (isCarryingMinerals()) {
		agent->Actions()->UnitCommand(&unit, sc2::ABILITY_ID::HARVEST_RETURN, &townhall->unit);
		agent->Actions()->UnitCommand(&unit, sc2::ABILITY_ID::GENERAL_MOVE, mineral_target->pos, true);
	}
	else {
		agent->Actions()->UnitCommand(&unit, sc2::ABILITY_ID::GENERAL_MOVE, mineral_target->pos);
	}
	return true;


}

bool Mob::grabNearbyMineralHarvester(BasicSc2Bot* agent, bool grab_from_gas, bool grab_from_other_townhall) {
	// called from a townhall to grab a nearby mob from gas and assign to minerals
	assert(hasFlag(FLAGS::IS_TOWNHALL));

	std::unordered_set<Mob*> nearby_workers = agent->mobH->filterByFlag(agent->mobH->getMobs(), FLAGS::IS_WORKER);
	if (nearby_workers.empty()) {
		return false;
	}

	if (!grab_from_other_townhall) {
		nearby_workers = agent->mobH->filterByFlag(nearby_workers, FLAGS::IS_MINERAL_GATHERER, false);
	}
	else {
		std::unordered_set<Mob*> filtered;
		for (auto w : nearby_workers) {
			if (w->isHarvestingMinerals()) {
				if (w->getTownhallForMinerals() != this) {
					filtered.insert(w);
				}
			}
			else {
				filtered.insert(w);
			}
		}
		nearby_workers = filtered;
	}

	if (nearby_workers.empty()) {
		return false;
	}

	std::unordered_set<Mob*> unassigned = agent->mobH->filterByFlag(nearby_workers, FLAGS::IS_GAS_GATHERER, false);
	unassigned = agent->mobH->filterByFlag(unassigned, FLAGS::IS_MINERAL_GATHERER, false);
	if (!unassigned.empty()) {
		if (!grab_from_gas && !grab_from_other_townhall) {
			return false;
		}
		nearby_workers = unassigned;
	}
	if (!grab_from_other_townhall) {
		nearby_workers = Directive::filterNearLocation(nearby_workers, unit.pos, 30.0F);
	}

	if (nearby_workers.empty()) {
		return false;
	}

	Mob* nearest = Directive::getClosestToLocation(nearby_workers, unit.pos);

	if (nearest->isHarvestingGas()) {
		nearest->stopHarvestingGas();
	}

	nearest->setFlag(FLAGS::IS_MINERAL_GATHERER);

	nearest->setHarvestingMinerals(this);
	nearest->setAssignedLocation(unit.pos);
	Directive directive_get_minerals(Directive::DEFAULT_DIRECTIVE, Directive::GET_MINERALS_NEAR_LOCATION, nearest->unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, unit.pos);
	agent->storeDirective(directive_get_minerals);
	Directive* dir = agent->getLastStoredDirective();
	nearest->assignDefaultDirective(agent, dir);
	const sc2::Unit* mineral_target = agent->locH->getNearestMineralPatch(unit.pos);
	if (grab_from_other_townhall) {
		if (nearest->isCarryingMinerals()) {
			auto town = agent->locH->getNearestTownhall(nearest->unit.pos);
			agent->Actions()->UnitCommand(&(nearest->unit), sc2::ABILITY_ID::HARVEST_RETURN, &town);
			agent->Actions()->UnitCommand(&(nearest->unit), sc2::ABILITY_ID::GENERAL_MOVE, mineral_target->pos, true);
		}
		else {
			agent->Actions()->UnitCommand(&(nearest->unit), sc2::ABILITY_ID::GENERAL_MOVE, mineral_target->pos);
		}
	}
	else {
		agent->Actions()->UnitCommand(&(nearest->unit), sc2::ABILITY_ID::HARVEST_GATHER, &mineral_target);
	}
	return true;


}

bool Mob::grabNearbyGasHarvester(BasicSc2Bot* agent) {
	// called from a gas structure to take a nearby mob off of minerals
	assert(hasFlag(FLAGS::IS_GAS_STRUCTURE));

	std::unordered_set<Mob*> nearby_workers = agent->mobH->filterByFlag(agent->mobH->getMobs(), FLAGS::IS_WORKER);
	nearby_workers = agent->mobH->filterByFlag(nearby_workers, FLAGS::IS_MINERAL_GATHERER);
	nearby_workers = Directive::filterNearLocation(nearby_workers, unit.pos, 30.0F);
	std::unordered_set<Mob*> filtered;
	std::copy_if(nearby_workers.begin(), nearby_workers.end(), std::inserter(filtered, filtered.begin()),
		[](Mob* m) { return (!m->isHarvestingGas()); });
	nearby_workers = filtered;
	
	if (nearby_workers.empty())
		return false;

	Mob* nearest = Directive::getClosestToLocation(nearby_workers, unit.pos);

	if (nearest->isHarvestingMinerals()) {
		nearest->stopHarvestingMinerals();
	}

	nearest->setFlag(FLAGS::IS_GAS_GATHERER);

	nearest->setHarvestingGas(this);

	Directive directive_get_gas(Directive::DEFAULT_DIRECTIVE, Directive::GET_GAS_NEAR_LOCATION, nearest->unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, unit.pos);
	agent->storeDirective(directive_get_gas);
	Directive* dir = agent->getLastStoredDirective();
	nearest->assignDefaultDirective(agent, dir);
	agent->Actions()->UnitCommand(&(nearest->unit), sc2::ABILITY_ID::HARVEST_GATHER, &unit);
	return true;
}


Directive* Mob::getDefaultDirective() {
	if (!has_default_directive) {
		return nullptr;
	}
	return default_directive;
}

sc2::Tag Mob::getTag() {
	return tag;
}