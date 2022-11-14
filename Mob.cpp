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
	}
	if (mobs_type == MOB::MOB_STRUCTURE) {
		// true flags
		flags.insert(FLAGS::IS_STRUCTURE);
	}
}

void Mob::initVars() {
	// initialize all flags to false
	tag = unit.tag;
	has_default_directive = false;
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

bool Mob::hasQueuedOrder() {
	return (queued_orders.size() > 0);
}

bool Mob::executeDefaultDirective(BotAgent* agent) {
	if (has_default_directive) {
		return default_directive->executeForUnit(agent, unit);
	}
	return false;
}

bool Mob::executeQueuedOrder(BotAgent* agent) {
	if (hasQueuedOrder()) {
		bool is_success = queued_orders.front().executeForUnit(agent, unit);
		queued_orders.erase(queued_orders.begin());
		return is_success;
	}
	return false;
}

sc2::Tag Mob::get_tag() {
	return tag;
}