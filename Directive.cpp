#include <cassert>
#include "sc2api/sc2_api.h"
#include "Directive.h"
#include "Agents.h"

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D assignee_location_,
	sc2::Point2D target_location_, float assignee_proximity_, float target_proximity_, std::unordered_set<FLAGS> flags_, sc2::Unit* unit_) {
	// genertic private constructor delegated by others
	// constructors which do not provide values for certain variables provide the listed default value instead

	locked = false;
	assignee = assignee_;
	action_type = action_type_;
	unit_type = unit_type_;						// default: UNIT_TYPEID::INVALID
	ability = ability_;							// default: ABILITY_TYPE::INVALID
	assignee_location = assignee_location_;		// default: (-1, -1)
	target_location = target_location_;			// default: (-1, -1)
	assignee_proximity = assignee_proximity_;   // default: -1.0f
	proximity = target_proximity_;				// default: -1.0f
	flags = flags_;								// default: empty
	target_unit = unit_;						// default: nullptr

	// assignee using match flags assigns multiple units, so force `allow_multiple = true`
	if (assignee == ASSIGNEE::MATCH_FLAGS || assignee == ASSIGNEE::MATCH_FLAGS_NEAR_LOCATION) {
		allow_multiple = true;
	}
	else {
		allow_multiple = false;
	}

	if (action_type == ACTION_TYPE::GET_MINERALS_NEAR_LOCATION) {

	}

}


Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_) :
	Directive(assignee_, action_type_, unit_type_, ability_, sc2::Point2D(-1, -1), 
		sc2::Point2D(-1, -1), -1.0f, -1.0f, std::unordered_set<FLAGS>(), nullptr) {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_) :
	Directive(assignee_, action_type_, unit_type_, ability_, sc2::Point2D(-1, -1),
		location_, -1.0f, proximity_, std::unordered_set<FLAGS>(), nullptr) {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Unit* target_) :
	Directive(assignee_, action_type_, unit_type_, ability_, sc2::Point2D(-1, -1),
		sc2::Point2D(-1, -1), -1.0f, -1.0f, std::unordered_set<FLAGS>(), target_) {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_) :
	Directive(assignee_, action_type_, sc2::UNIT_TYPEID::INVALID, ability_, sc2::Point2D(-1, -1),
		location_, -1.0f, proximity_, flags_, nullptr) {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_,
	sc2::Point2D assignee_location_, sc2::Point2D target_location_, float assignee_proximity_, float target_proximity_) :
	Directive(assignee_, action_type_, sc2::UNIT_TYPEID::INVALID, ability_, assignee_location_,
		target_location_, assignee_proximity_, target_proximity_, flags_, nullptr) {}

bool Directive::bundleDirective(Directive directive_) {
	if (!locked)
		directive_bundle.push_back(directive_);
	return !locked;
}

bool Directive::execute(BotAgent* agent) {
	const sc2::ObservationInterface* obs = agent->Observation();
	bool found_valid_unit = false; // ensure unit has been assigned before issuing order
	const sc2::AbilityData ability_data = obs->GetAbilityData()[(int)ability]; // various info about the ability
	sc2::QueryInterface* query_interface = agent->Query(); // used to query data
	std::unordered_set<Mob*> mobs = agent->get_mobs(); // unordered_set of all friendly units
	Mob* mob; // used to store temporary mob

	// ensure proper variables are set for the specified ASSIGNEE and ACTION_TYPE

	if (assignee == ASSIGNEE::UNIT_TYPE || assignee == ASSIGNEE::UNIT_TYPE_NEAR_LOCATION) {
		assert(unit_type != sc2::UNIT_TYPEID::INVALID);
	}

	if (assignee == ASSIGNEE::MATCH_FLAGS || assignee == ASSIGNEE::MATCH_FLAGS_NEAR_LOCATION) {
		assert(flags.size() > 0);
	}

	if (assignee == ASSIGNEE::UNIT_TYPE_NEAR_LOCATION || assignee == ASSIGNEE::MATCH_FLAGS_NEAR_LOCATION) {
		assert(assignee_location != sc2::Point2D(-1, -1));
	}

	if (action_type == ACTION_TYPE::EXACT_LOCATION || action_type == ACTION_TYPE::NEAR_LOCATION ||
		action_type == ACTION_TYPE::GET_GAS_NEAR_LOCATION || action_type == ACTION_TYPE::GET_MINERALS_NEAR_LOCATION ||
		action_type == ACTION_TYPE::TARGET_UNIT_NEAR_LOCATION) {
		assert(target_location != sc2::Point2D(-1, -1));
	}

	if (assignee == ASSIGNEE::UNIT_TYPE || assignee == ASSIGNEE::UNIT_TYPE_NEAR_LOCATION) {
		if (action_type == ACTION_TYPE::EXACT_LOCATION || action_type == ACTION_TYPE::NEAR_LOCATION) {
			
			if (ability == sc2::ABILITY_ID::BUILD_ASSIMILATOR ||
				ability == sc2::ABILITY_ID::BUILD_EXTRACTOR ||
				ability == sc2::ABILITY_ID::BUILD_REFINERY) {
				// find a geyser near target location to build the structure on
				return execute_build_gas_structure(agent);
			}

			if (ability == sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST) {
				return execute_protoss_nexus_chronoboost(agent);
			}

			return execute_order_for_unit_type_with_location(agent);
		}
		if (action_type == SIMPLE_ACTION) {
			return execute_simple_action_for_unit_type(agent);
		}
	}

	if (assignee == MATCH_FLAGS || assignee == MATCH_FLAGS_NEAR_LOCATION) {
		return execute_match_flags(agent);
	}
	
	return false;
}

bool Directive::executeForMob(BotAgent* agent, Mob* mob_) {
	// used to assign an order to a specific unit
	Mob* mob = &agent->getMob(mob_->unit);
	if (!mob) {
		return false;
	}

	if (action_type == GET_MINERALS_NEAR_LOCATION) {

		// Note: this order cannot have further queued directives attached,
		//       as harvesting continues indefinitely
		
		if (mob->is_carrying_minerals()) {
			// if unit is carrying minerals, return them to the townhall instead
			const sc2::Unit* townhall = agent->FindNearestTownhall(target_location);
			if (!townhall)
				return false;
			/* ORDER IS EXECUTED */
			agent->set_mob_idle(mob, false);
			return issueOrder(agent, mob, false, sc2::ABILITY_ID::HARVEST_RETURN);
			/* * * * * * * * * * */
		}

		const sc2::Unit* mineral_target = agent->FindNearestMineralPatch(target_location);

		if (!mineral_target) {
			return false;
		}

		/* ORDER IS EXECUTED */
		agent->set_mob_idle(mob, false);
		return issueOrder(agent, mob, mineral_target);
		//agent->Actions()->UnitCommand(&mob->unit, ability, mineral_target);
		/* * * * * * * * * * */
	}
	if (action_type == GET_GAS_NEAR_LOCATION) {

		// Note: this order cannot have further queued directives attached
		//       as harvesting continues indefinitely
		
		if (mob->is_carrying_gas()) {
			// if unit is carrying gas, return them to the townhall instead
			const sc2::Unit* townhall = agent->FindNearestTownhall(target_location);
			if (!townhall)
				return false;

			/* ORDER IS EXECUTED */
			agent->set_mob_idle(mob, false);
			return issueOrder(agent, mob, false, sc2::ABILITY_ID::HARVEST_RETURN);
			/* * * * * * * * * * */
		}

		const sc2::Unit* gas_target = agent->FindNearestGasStructure(target_location);
		if (!gas_target) {
			return false;
		}
		/* ORDER IS EXECUTED */
		agent->set_mob_idle(mob, false);
		return issueOrder(agent, mob, gas_target);
		//agent->Actions()->UnitCommand(&mob->unit, sc2::ABILITY_ID::HARVEST_GATHER, gas_target);
		/* * * * * * * * * * */
	}
	if (action_type == SIMPLE_ACTION) {

		/* ORDER IS EXECUTED */
		agent->set_mob_idle(mob, false);
		//agent->Actions()->UnitCommand(&mob->unit, ability);
		if (have_bundle()) mob->bundle_directives(directive_bundle);
		return issueOrder(agent, mob);
		/* * * * * * * * * * */
	}
	if (action_type == EXACT_LOCATION || action_type == NEAR_LOCATION) {
		sc2::Point2D location = target_location;

		if (action_type == NEAR_LOCATION)
			location = uniform_random_point_in_circle(target_location, proximity);

		/* ORDER IS EXECUTED */
		agent->set_mob_idle(mob, false);
		//agent->Actions()->UnitCommand(&mob->unit, ability, location);
		if (have_bundle()) mob->bundle_directives(directive_bundle);
		return issueOrder(agent, mob, location);
		/* * * * * * * * * * */
	}
	return false;
}

bool Directive::_generic_issueOrder(BotAgent* agent, std::unordered_set<Mob*> mobs_, sc2::Point2D target_loc_, const sc2::Unit* target_unit_, bool queued_, sc2::ABILITY_ID ability_) {
	
	// if no ability override specified
	if (ability_ == USE_DEFINED_ABILITY)
		ability_ = ability;

	// handle case where more than one mob is being assigned the order
	if (mobs_.size() > 1) {
		sc2::Units units;
		for (auto m_ : mobs_) {
			units.push_back(&(m_->unit));
		}

		// no target is specified
		if (target_loc_ == sc2::Point2D(-1, -1) && target_unit_ == nullptr) {
			agent->Actions()->UnitCommand(units, ability, queued_);
			return true;
		}

		// target location is specified
		if (target_loc_ != sc2::Point2D(-1, -1) && target_unit_ == nullptr) {
			agent->Actions()->UnitCommand(units, ability, target_loc_, queued_);
			return true;
		}

		// target unit is specified
		if (target_loc_ == sc2::Point2D(-1, -1) && target_unit_ != nullptr) {
			agent->Actions()->UnitCommand(units, ability, target_unit_, queued_);
			return true;
		}
	}
	else {
		// handle case where only one mob is being assigned the order
				// no target is specified

		const sc2::Unit* unit = &(*mobs_.begin())->unit;

		if (target_loc_ == sc2::Point2D(-1, -1) && target_unit_ == nullptr) {
			agent->Actions()->UnitCommand(unit, ability, queued_);
			return true;
		}

		// target location is specified
		if (target_loc_ != sc2::Point2D(-1, -1) && target_unit_ == nullptr) {
			agent->Actions()->UnitCommand(unit, ability, target_loc_, queued_);
			return true;
		}

		// target unit is specified
		if (target_loc_ == sc2::Point2D(-1, -1) && target_unit_ != nullptr) {
			agent->Actions()->UnitCommand(unit, ability, target_unit_, queued_);
			return true;
		}
	}
	
}

bool Directive::issueOrder(BotAgent* agent, Mob* mob_, bool queued_, sc2::ABILITY_ID ability_) {
	return _generic_issueOrder(agent, std::unordered_set<Mob*>{ mob_ }, sc2::Point2D(-1, -1), nullptr, queued_, ability_);
}

bool Directive::issueOrder(BotAgent* agent, Mob* mob_, sc2::Point2D target_loc_, bool queued_, sc2::ABILITY_ID ability_) {
	return _generic_issueOrder(agent, std::unordered_set<Mob*>{ mob_ }, target_loc_, nullptr, queued_, ability_);
}

bool Directive::issueOrder(BotAgent* agent, Mob* mob_, const sc2::Unit* target_unit_, bool queued_, sc2::ABILITY_ID ability_) {
	return _generic_issueOrder(agent, std::unordered_set<Mob*>{ mob_ }, sc2::Point2D(-1, -1), target_unit_, queued_, ability_);
}

bool Directive::issueOrder(BotAgent* agent, std::unordered_set<Mob*> mobs_, bool queued_, sc2::ABILITY_ID ability_) {
	return _generic_issueOrder(agent, mobs_, sc2::Point2D(-1, -1), nullptr, queued_, ability_);
}

bool Directive::issueOrder(BotAgent* agent, std::unordered_set<Mob*> mobs_, sc2::Point2D target_loc_, bool queued_, sc2::ABILITY_ID ability_) {
	return _generic_issueOrder(agent, mobs_, target_loc_, nullptr, queued_, ability_);
}

bool Directive::issueOrder(BotAgent* agent, std::unordered_set<Mob*> mobs_, const sc2::Unit* target_unit_, bool queued_, sc2::ABILITY_ID ability_) {
	return _generic_issueOrder(agent, mobs_, sc2::Point2D(-1, -1), target_unit_, queued_, ability_);
}

bool Directive::setDefault() {
	// a default directive is something that a unit performs when it has no actions
	// usually used for workers to return to gathering after building/defending
	if (!locked)
		assignee = DEFAULT_DIRECTIVE;
	return !locked;
}

std::unordered_set<sc2::Tag> Directive::getAssignedMobTags() {
	return assigned_mob_tags;
}

bool Directive::hasAssignedMob() {
	return !assigned_mob_tags.empty();
}

bool Directive::assignMob(Mob* mob_) {
	// adds a mob to this directive's assigned mobs
	// returns false if directive does not allow multiple mobs and already has one
	if (!allow_multiple && !assigned_mob_tags.empty()) {
		return false;
	}
	assigned_mob_tags.insert(mob_->get_tag());
	return true;
}

sc2::Point2D Directive::uniform_random_point_in_circle(sc2::Point2D center, float radius) {
	// given the center point and radius, return a uniform random point within the circle

	float r = radius * sqrt(sc2::GetRandomScalar());
	float theta = sc2::GetRandomScalar() * 2 * M_PI;
	float x = center.x + r * cos(theta);
	float y = center.y + r * sin(theta);
	return sc2::Point2D(x, y);
}

bool Directive::execute_simple_action_for_unit_type(BotAgent* agent) {
	// perform an action that does not require a target unit or point

	std::unordered_set<Mob*> mobs = agent->get_mobs(); // unordered_set of all friendly units
	Mob* mob;

	// filter idle units which match unit_type
	mobs = filter_by_unit_type(mobs, unit_type);
	mobs = filter_idle(mobs);

	if (assignee == UNIT_TYPE_NEAR_LOCATION) {
		mobs = filter_near_location(mobs, assignee_location, assignee_proximity);
	}

	if (mobs.size() == 0)
		return false;

	mob = get_random_mob_from_set(mobs);

	/* ORDER IS EXECUTED */
	agent->set_mob_idle(mob, false);
	agent->Actions()->UnitCommand(&mob->unit, ability);
	if (have_bundle()) mob->bundle_directives(directive_bundle);
	return true;
	/* * * * * * * * * * */


}

bool Directive::execute_build_gas_structure(BotAgent* agent) {
	// perform the necessary actions to have a gas structure built closest to the specified target_location

	std::unordered_set<Mob*> mobs = agent->get_mobs(); // vector of all friendly units
	Mob* mob; // used to store temporary mob
	bool found_valid_unit = false;

	// find a vespene geyser near the location and build on it
	const sc2::Unit* geyser_target = agent->FindNearestGeyser(target_location);
	sc2::Point2D location = geyser_target->pos;

	if (assignee == UNIT_TYPE_NEAR_LOCATION) {
		mobs = filter_near_location(mobs, assignee_location, assignee_proximity);
	}

	mobs = filter_by_unit_type(mobs, unit_type);

	// there are no valid units to perform this action
	if (mobs.size() == 0)
		return false;

	// check if any of this unit type are already building a gas structure
	if (is_any_executing_order(mobs, ability))
		return false;
	
	// pick valid mob to execute order, closest to geyser
	mob = get_closest_to_location(mobs, target_location);

	if (!mob)
		return false;

	/* ORDER IS EXECUTED */
	agent->set_mob_idle(mob, false);
	mob->set_flag(FLAGS::BUILDING_GAS);
	mob->set_flag(FLAGS::IS_BUILDING_STRUCTURE);
	agent->Actions()->UnitCommand(&(mob->unit), ability, geyser_target, true);
	if (have_bundle()) mob->bundle_directives(directive_bundle);
	return true;
	/* * * * * * * * * * */


}

bool Directive::execute_protoss_nexus_chronoboost(BotAgent* agent) {
	// this is more complex than it originally seemed
	// must find the clostest nexus to the given location
	// that has the chronoboost ability ready
	// then find a structure that would benefit from it

	std::unordered_set<Mob*> mobs = agent->get_mobs(); // vector of all friendly units
	Mob* mob; // used to store temporary mob
	Mob* chrono_target;
	std::unordered_set<Mob*> mobs_filter1;
	static const sc2::Unit* unit_to_target;

	if (assignee == UNIT_TYPE_NEAR_LOCATION) {
		mobs = filter_near_location(mobs, assignee_location, assignee_proximity);
	}

	// first get list of Mobs matching type
	mobs = filter_by_unit_type(mobs, unit_type);

	// then filter by those with ability available
	std::unordered_set<Mob*> mobs_filter;
	std::copy_if(mobs.begin(), mobs.end(), std::inserter(mobs_filter, mobs_filter.begin()),
		[agent, this](Mob* m) { return agent->can_unit_use_ability(m->unit, ability); });
	mobs = mobs_filter;
	
	// return false if no structures exist with chronoboost ready to cast
	int num_units = mobs.size();
	if (num_units == 0)
		return false;

	// then pick the one closest to location
	mob = get_closest_to_location(mobs, target_location);

	// return false if mob has not been assigned above
	if (!mob)
		return false;

	// get all structures
	std::unordered_set<Mob*> structures = agent->filter_by_flag(agent->get_mobs(), FLAGS::IS_STRUCTURE);
	std::unordered_set<Mob*> structures_with_orders; 

	// look for buildings that are doing something
	std::copy_if(structures.begin(), structures.end(), std::inserter(structures_with_orders, structures_with_orders.begin()),
		[this](Mob* m) { return (m->unit.orders).size() > 0; });

	if (structures_with_orders.size() == 0)
		return false;

	// then pick one of THESE closest to location
	chrono_target = get_closest_to_location(structures_with_orders, target_location);

	// return false if there is nothing worth casting chronoboost on
	if (!chrono_target) {
		return false;
	}

	/* ORDER IS EXECUTED */
	agent->set_mob_idle(mob, false);
	agent->Actions()->UnitCommand(&mob->unit, ability, &chrono_target->unit, true);
	if (have_bundle()) mob->bundle_directives(directive_bundle);
	return true;
	/* * * * * * * * * * */
}

bool Directive::execute_match_flags(BotAgent* agent) {
	// issue an order to units matching the provided flags

	std::unordered_set<Mob*> mobs = agent->get_mobs(); // vector of all friendly units
	std::unordered_set<Mob*> matching_mobs = agent->filter_by_flags(mobs, flags);

	// get only units near the assignee_location parameter
	if (assignee == MATCH_FLAGS_NEAR_LOCATION) {
		matching_mobs = filter_near_location(matching_mobs, assignee_location, assignee_proximity);
	}

	// no units match the condition(s)
	if (matching_mobs.size() == 0) {
		return false;
	}

	bool found_valid_unit = false;
	sc2::Point2D location = target_location;
	sc2::Units units;
	std::unordered_set<Mob*> filtered_mobs;
	if (action_type == ACTION_TYPE::EXACT_LOCATION || action_type == ACTION_TYPE::NEAR_LOCATION) {
		for (Mob* m : matching_mobs) {
			if (action_type == ACTION_TYPE::NEAR_LOCATION) {
				location = uniform_random_point_in_circle(target_location, proximity);
			}
			// Unit has no orders
			if ((m->unit.orders).size() == 0) {
				found_valid_unit = true;
				filtered_mobs.insert(m);
				units.push_back(&m->unit);
			}
		}
	}
	if (!found_valid_unit)
		return false;

	/* ORDER IS EXECUTED */
	for (auto m : filtered_mobs) agent->set_mob_idle(m, false);
	Mob* mob_with_bundle = get_random_mob_from_set(filtered_mobs);
	agent->Actions()->UnitCommand(units, ability, location);
	if (have_bundle()) mob_with_bundle->bundle_directives(directive_bundle);
	return true;
	/* * * * * * * * * * */
}

bool Directive::execute_order_for_unit_type_with_location(BotAgent* agent) {
	const sc2::AbilityData ability_data = agent->Observation()->GetAbilityData()[(int)ability]; // various info about the ability
	sc2::QueryInterface* query_interface = agent->Query(); // used to query data
	sc2::Point2D location = target_location;
	std::unordered_set<Mob*> mobs = agent->get_mobs();
	Mob* mob = nullptr;

	if (action_type == ACTION_TYPE::NEAR_LOCATION) {
		location = uniform_random_point_in_circle(target_location, proximity);
	}

	mobs = filter_by_unit_type(mobs, unit_type);

	// check if a unit of this type is already executing this order
	if (is_any_executing_order(mobs, ability)) {
		return false;
	}

	// unit of type does not exist
	if (mobs.size() == 0) {
		return false;
	}

	mobs = filter_by_has_ability(agent, mobs, ability);
	if (mobs.size() == 0) {
		return false;
	}

    
	// if order is a build structure order, ensure a valid placement location
	if (ability_data.is_building) {
		int i = 0;
		while (!query_interface->Placement(ability, location)) {
			location = uniform_random_point_in_circle(target_location, proximity);
			i++;
			if (i > 20) {
				// can't find a suitable spot to build
				return false;
			}
		}

		// filter only those not currently building a structure
		mobs = filter_not_building_structure(agent, mobs);

		if (mobs.size() == 0) {
			return false;
		}
	}

	// get closest matching unit to target location
	mob = get_closest_to_location(mobs, target_location);

	if (!mob) {
		return false;
	}

	// if the ability does not require a target location
	if (ability_data.target == sc2::AbilityData::Target::None) {
		
		/* ORDER IS EXECUTED */
		agent->set_mob_idle(mob, false);
		agent->Actions()->UnitCommand(&mob->unit, ability);
		if (have_bundle()) mob->bundle_directives(directive_bundle);
		return true;
	}

	// if the ability requires a target unit
	if (ability_data.target == sc2::AbilityData::Target::Unit) {
		std::cerr << ability_data.friendly_name << " requires a target unit." << std::endl;
		return false;
	}

	// else, the ability requires a target location

	/* ORDER IS EXECUTED */
	if (ability_data.is_building)
		mob->set_flag(FLAGS::IS_BUILDING_STRUCTURE);
	agent->set_mob_idle(mob, false);
	agent->Actions()->UnitCommand(&mob->unit, ability, location);
	if (have_bundle()) mob->bundle_directives(directive_bundle);
	return true;
	/* * * * * * * * * * */
}

bool Directive::have_bundle() {
	// if this directive has other directives bundled with it
	return (directive_bundle.size() > 0);
}

bool Directive::is_building_structure(BotAgent* agent, Mob* mob_) {
	sc2::QueryInterface* query_interface = agent->Query();
	const sc2::ObservationInterface* obs = agent->Observation();
	std::vector<sc2::AbilityData> ability_data = obs->GetAbilityData();

	// check if unit is on its way to build a structure
	for (const auto& order : mob_->unit.orders) {
		sc2::ABILITY_ID order_ability = order.ability_id;
		if (ability_data[(int)order_ability].is_building) {
			return true;
		}
	}
	
	// check if unit is in the process of building a structure
	std::vector<sc2::AvailableAbility> abilities = (query_interface->GetAbilitiesForUnit(&mob_->unit)).abilities;
	bool match = false;
	for (auto a : abilities) {
		if (a.ability_id == sc2::ABILITY_ID::HALT) {
			return true;
		}
	}
	return match;
}

bool Directive::is_any_executing_order(std::unordered_set<Mob*> mobs_set, sc2::ABILITY_ID ability_) {
	// check if any Mob in the vector is already executing the specified order
	for (Mob* m : mobs_set) {
		for (const auto& order : m->unit.orders) {
			if (order.ability_id == ability_)
				return true;
		}
	}
	return false;
}

Mob* Directive::get_closest_to_location(std::unordered_set<Mob*> mobs_set, sc2::Point2D pos_) {
	// return a pointer to the Mob object closest to a location

	float lowest_distance = std::numeric_limits<float>::max();
	Mob* closest_sm = nullptr;
	for (Mob* m : mobs_set) {
		float dist = sc2::DistanceSquared2D(m->unit.pos, pos_);
		if (dist < lowest_distance) {
			closest_sm = m;
			lowest_distance = dist;
		}
	}
	return closest_sm;
}

bool Directive::allowMultiple(bool is_true) {
	// allow this directive to be assigned to more than one Mob
	if (!locked) {
		allow_multiple = is_true;
		return true;
	}
	return false;
}

std::unordered_set<Mob*> Directive::filter_near_location(std::unordered_set<Mob*> mobs_set, sc2::Point2D pos_, float radius_) {
	// filters a vector of Mob* by only those within the specified distance to location
	float sq_dist = pow(radius_, 2);

	std::unordered_set<Mob*> filtered_mobs;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered_mobs, filtered_mobs.begin()),
		[sq_dist, pos_](Mob* m) { return (
			sc2::DistanceSquared2D(m->unit.pos, pos_) <= sq_dist);
		});
	return filtered_mobs;
}

std::unordered_set<Mob*> Directive::filter_not_building_structure(BotAgent* agent, std::unordered_set<Mob*> mobs_set) {
	// returns only units that are not currently constructing a structure
	
	std::unordered_set<Mob*> filtered;

	/*
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered, filtered.begin()),
		[this, agent](Mob* m) { return !is_building_structure(agent, m); });
		*/

	filtered = agent->filter_by_flag(mobs_set, FLAGS::IS_BUILDING_STRUCTURE, false);
	return filtered;

}

std::unordered_set<Mob*> Directive::filter_by_has_ability(BotAgent* agent, std::unordered_set<Mob*> mobs_set, sc2::ABILITY_ID ability_) {
	// get only mobs from set that have a specified ability

	std::unordered_set<Mob*> mobs_filter;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(mobs_filter, mobs_filter.begin()),
		[agent, this](Mob* m) { return agent->can_unit_use_ability(m->unit, ability); });

	return mobs_filter;
}

std::unordered_set<Mob*> Directive::filter_by_unit_type(std::unordered_set<Mob*> mobs_set, sc2::UNIT_TYPEID unit_type_) {
	std::unordered_set<Mob*> filtered;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered, filtered.begin()),
		[this](Mob* m) { return m->unit.unit_type == unit_type; });
	return filtered;
}

std::unordered_set<Mob*> Directive::filter_idle(std::unordered_set<Mob*> mobs_set) {
	std::unordered_set<Mob*> filtered;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered, filtered.begin()),
		[this](Mob* m) { return (m->unit.orders).size() == 0; });
	return filtered;
}

Mob* Directive::get_random_mob_from_set(std::unordered_set<Mob*> mob_set) {
	int index = rand() % mob_set.size();
	auto it = mob_set.begin();
	for (int i = 0; i < index; i++)
	{
		it++;
	}
	return *it;
}

void Directive::lock() {
	// prevent further modification to this
	locked = true;
}

bool Directive::allowsMultiple() {
	// check if this directive allows multiple mobs to be assigned at once
	return allow_multiple;
}

