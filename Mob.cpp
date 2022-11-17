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
	birth_location = unit.pos;
	home_location = unit.pos;
	assigned_location = unit.pos;
	std::unordered_set<FLAGS> flags;
}

bool Mob::is_idle() {
	return ((unit.orders).size() == 0);
}

bool Mob::has_flag(FLAGS flag) {
	return (flags.find(flag) != flags.end());
}

void Mob::assignDirective(Directive directive_) {
	if (has_default_directive) {
		delete default_directive;
	}
	directive_.setDefault(); // change directive type to default directive
	default_directive = new Directive(directive_);
	default_directive->lock();
	has_default_directive = true;
}

bool Mob::hasDefaultDirective() {
	return has_default_directive;
}

bool Mob::hasBundledDirective() {
	return has_bundled_directive;
}


bool Mob::executeDefaultDirective(BotAgent* agent) {
	if (has_default_directive) {
		return default_directive->executeForUnit(agent, unit);
	}
	return false;
}

void Mob::bundle_directives(std::vector<Directive> dir_vec) {
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


void Mob::set_flag(FLAGS flag) {
	flags.insert(flag);
}

void Mob::remove_flag(FLAGS flag) {
	flags.erase(flag);
}

Directive Mob::popBundledDirective() {
	// get the directive bundled on this mob and simultaneously remove it from this mob
	Directive bundled = *bundled_directive;
	delete bundled_directive;
	has_bundled_directive = false;
	return bundled;
}

bool Mob::is_carrying_minerals() {
	// return true if a unit is carrying minerals

	std::vector<sc2::BuffID> unit_buffs = unit.buffs;
	std::vector<sc2::BuffID> mineral_buffs{
		sc2::BUFF_ID::CARRYHIGHYIELDMINERALFIELDMINERALS,
		sc2::BUFF_ID::CARRYMINERALFIELDMINERALS
	};

	return std::find_first_of(unit_buffs.begin(), unit_buffs.end(), mineral_buffs.begin(), mineral_buffs.end()) != unit_buffs.end();
}

bool Mob::is_carrying_gas() {
	// return true if a unit is carrying gas

	std::vector<sc2::BuffID> unit_buffs = unit.buffs;
	std::vector<sc2::BuffID> gas_buffs{
		sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGAS,
		sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASPROTOSS,
		sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGASZERG
	};

	return std::find_first_of(unit_buffs.begin(), unit_buffs.end(), gas_buffs.begin(), gas_buffs.end()) != unit_buffs.end();
}

void Mob::set_home_location(sc2::Point2D location) {
	home_location = location;
}

void Mob::set_assigned_location(sc2::Point2D location) {
	assigned_location = location;
}

sc2::Point2D Mob::get_birth_location() {
	return birth_location;
}

sc2::Point2D Mob::get_home_location() {
	return home_location;
}

sc2::Point2D Mob::get_assigned_location() {
	return assigned_location;
}

std::unordered_set<FLAGS> Mob::get_flags() {
	return flags;
}

sc2::Tag Mob::get_tag() {
	return tag;
}