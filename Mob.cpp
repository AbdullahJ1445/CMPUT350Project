#include "Mob.h"

Mob::Mob(const sc2::Unit& unit_, MOB mobs_type) : unit(unit_) {
	initVars();

	if (mobs_type == MOB::MOB_WORKER) {
		// default all trained workers to be mineral gatherers for now
		// true flags
		flags.insert(FLAGS::IS_WORKER);
		flags.insert(FLAGS::IS_MINERAL_GATHERER);
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
	has_default_directive = false;
	has_bundled_directive = false;
	has_current_directive = false;
	is_harvesting_gas = false;
	birth_location = unit.pos;
	home_location = unit.pos;
	assigned_location = unit.pos;
	current_directive = nullptr;
	std::unordered_set<FLAGS> flags;
	std::unordered_set<Mob*> harvesters;
	gas_structure_harvested = nullptr;
}

bool Mob::isIdle() {
	return ((unit.orders).size() == 0);
}

bool Mob::hasFlag(FLAGS flag) {
	return (flags.find(flag) != flags.end());
}

void Mob::assignDefaultDirective(Directive directive_) {
	if (has_default_directive) {
		delete default_directive;
	}
	directive_.setDefault(); // change directive type to default directive
	default_directive = new Directive(directive_);
	default_directive->lock();
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

bool Mob::isHarvestingGas() {
	return is_harvesting_gas;
}

void Mob::stopHarvestingGas() {
	// set this unit to stop harvesting a gas structure
	// does not change the default directive (yet)

	if (is_harvesting_gas) {
		if (gas_structure_harvested != nullptr) {
			gas_structure_harvested->removeHarvester(this);
		}
		gas_structure_harvested = nullptr;
		is_harvesting_gas = false;
	}
}

void Mob::addHarvester(Mob* mob_) {
	assert(mob_->hasFlag(FLAGS::IS_WORKER));
	harvesters.insert(mob_);
}

void Mob::removeHarvester(Mob* mob_) {
	// remove harvester from this gas structure

	//assert(hasFlag(FLAGS::IS_GAS_STRUCTURE));

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

int Mob::getHarvesterCount() {
	return harvesters.size();
}

bool Mob::grabNearbyGasHarvester(BasicSc2Bot* agent) {
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

	nearest->removeFlag(FLAGS::IS_MINERAL_GATHERER);
	nearest->setFlag(FLAGS::IS_GAS_GATHERER);

	nearest->setHarvestingGas(this);

	Directive directive_get_gas(Directive::DEFAULT_DIRECTIVE, Directive::GET_GAS_NEAR_LOCATION, nearest->unit.unit_type, sc2::ABILITY_ID::HARVEST_GATHER, unit.pos);
	nearest->assignDefaultDirective(directive_get_gas);
	agent->Actions()->UnitCommand(&(nearest->unit), sc2::ABILITY_ID::HARVEST_GATHER, &unit);
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