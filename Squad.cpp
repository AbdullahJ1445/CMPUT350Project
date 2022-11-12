#include "Squad.h"


SquadMember::SquadMember(const sc2::Unit& unit_, SQUAD squad_type) : unit(unit_) {
	initVars();

	if (squad_type == SQUAD::SQUAD_WORKER) {
		// default all trained workers to be mineral gatherers for now
		// true flags
		flags.insert(FLAGS::IS_WORKER);
		flags.insert(FLAGS::IS_MINERAL_GATHERER);
	}
	if (squad_type == SQUAD::SQUAD_TOWNHALL) {
		// true flags
		flags.insert(FLAGS::IS_STRUCTURE);
		flags.insert(FLAGS::IS_TOWNHALL);
		flags.insert(FLAGS::IS_SUPPLY);
	}
	if (squad_type == SQUAD::SQUAD_STRUCTURE) {
		// true flags
		flags.insert(FLAGS::IS_STRUCTURE);
	}
}

void SquadMember::initVars() {
	// initialize all flags to false
	has_default_directive = false;
	group_id = -1;
	birth_location = unit.pos;
	home_location = unit.pos;
	assigned_location = unit.pos;
	std::unordered_set<FLAGS> flags;
}

bool SquadMember::is_idle() {
	return ((unit.orders).size() == 0);
}

bool SquadMember::has_flag(FLAGS flag) {
	return (flags.find(flag) != flags.end());
}

void SquadMember::assignDirective(Directive directive_) {
	if (has_default_directive) {
		delete default_directive;
	}
	directive_.setDefault(); // change directive type to default directive
	default_directive = new Directive(directive_);
	has_default_directive = true;
}

bool SquadMember::hasDefaultDirective() {
	return has_default_directive;
}

bool SquadMember::executeDefaultDirective(BotAgent* agent, const sc2::ObservationInterface* obs) {
	if (has_default_directive) {
		return default_directive->executeForUnit(agent, obs, unit);
	}
	return false;
}


/*
enum class FLAGS {
	IS_STRUCTURE,
	IS_TOWNHALL,
	IS_SUPPLY,
	IS_WORKER,
	IS_MINERAL_GATHERER,
	IS_GAS_GATHERER,
	IS_DEFENSE,
	IS_PROXY,
	IS_SIEGE,
	IS_ALERT,
	IS_ATTACKER,
	IS_ATTACKING,
	IS_FLYING,
	IS_INVISIBLE,
};
*/