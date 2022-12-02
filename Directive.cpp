#include <cassert>
#include "sc2api/sc2_api.h"
#include "Directive.h"
#include "BasicSc2Bot.h"
#include "Mob.h"

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D assignee_location_,
	sc2::Point2D target_location_, float assignee_proximity_, float target_proximity_, std::unordered_set<FLAGS> flags_, sc2::Unit* unit_, std::string group_name_) {
	// genertic private constructor delegated by others
	// constructors which do not provide values for certain variables provide the listed default value instead

	locked = false;
	static size_t id_ = 0;
	id = id_++;
	assignee = assignee_;
	action_type = action_type_;
	unit_type = unit_type_;						// default: UNIT_TYPEID::INVALID
	ability = ability_;							// default: ABILITY_TYPE::INVALID
	assignee_location = assignee_location_;		// default: INVALID_POINT
	target_location = target_location_;			// default: INVALID_POINT
	assignee_proximity = assignee_proximity_;   // default: INVALID_RADIUS
	proximity = target_proximity_;				// default: INVALID_RADIUS
	flags = flags_;								// default: empty
	target_unit = unit_;						// default: nullptr
	update_assignee_location = false;
	update_target_location = false;
	group_name = group_name_;
	continuous_update = false;
	target_update_iter_id = 0;
	assignee_update_iter_id = 0;
	debug = false;


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


Directive::Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, float assignee_proximity_) : 
	Directive(assignee_, action_type_, unit_type_, sc2::ABILITY_ID::INVALID, assignee_location_,
		INVALID_POINT, assignee_proximity_, INVALID_RADIUS, std::unordered_set<FLAGS>(), nullptr, "") {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_) :
	Directive(assignee_, action_type_, unit_type_, ability_, INVALID_POINT, 
		INVALID_POINT, INVALID_RADIUS, INVALID_RADIUS, std::unordered_set<FLAGS>(), nullptr, "") {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_) :
	Directive(assignee_, action_type_, unit_type_, ability_, INVALID_POINT,
		location_, INVALID_RADIUS, proximity_, std::unordered_set<FLAGS>(), nullptr, "") {}

Directive::Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, float assignee_proximity_) :
	Directive(assignee_, action_type_, unit_type_, ability_, assignee_location_, INVALID_POINT, assignee_proximity_, INVALID_RADIUS, std::unordered_set<FLAGS>(), nullptr, "") {}

Directive::Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D target_location_, float assignee_proximity_, float target_proximity_) :
	Directive(assignee_, action_type_, unit_type_, ability_, assignee_location_, target_location_, assignee_proximity_, target_proximity_, std::unordered_set<FLAGS>(), nullptr, "") {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Unit* target_) :
	Directive(assignee_, action_type_, unit_type_, ability_, INVALID_POINT,
		INVALID_POINT, INVALID_RADIUS, INVALID_RADIUS, std::unordered_set<FLAGS>(), target_, "") {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_) :
	Directive(assignee_, action_type_, sc2::UNIT_TYPEID::INVALID, ability_, INVALID_POINT,
		location_, INVALID_RADIUS, proximity_, flags_, nullptr, "") {}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_,
	sc2::Point2D assignee_location_, sc2::Point2D target_location_, float assignee_proximity_, float target_proximity_) :
	Directive(assignee_, action_type_, sc2::UNIT_TYPEID::INVALID, ability_, assignee_location_,
		target_location_, assignee_proximity_, target_proximity_, flags_, nullptr, "") {}

Directive::Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, std::string group_name_, sc2::UNIT_TYPEID unit_type_, float assignee_proximity_) :
	Directive(assignee_, action_type_, unit_type_, sc2::ABILITY_ID::INVALID, assignee_location_, INVALID_POINT, assignee_proximity_, INVALID_RADIUS, std::unordered_set<FLAGS>(), nullptr, group_name_) {}



bool Directive::bundleDirective(Directive directive_) {
	if (!locked)
		directive_bundle.push_back(directive_);
	return !locked;
}

bool Directive::execute(BasicSc2Bot* agent) {
	const sc2::ObservationInterface* obs = agent->Observation();
	bool found_valid_unit = false; // ensure unit has been assigned before issuing order
	const sc2::AbilityData ability_data = obs->GetAbilityData()[(int)ability]; // various info about the ability
	sc2::QueryInterface* query_interface = agent->Query(); // used to query data
	std::unordered_set<Mob*> mobs = agent->mobH->getMobs(); // unordered_set of all friendly units
	Mob* mob; // used to store temporary mob
	if (update_assignee_location) {
		updateAssigneeLocation(agent);
	}

	if (update_target_location) {
		updateTargetLocation(agent);
		//std::cout << "order targetting " << target_location.x << "," << target_location.y << std::endl;
	}

	if (debug) {
		std::cout << "(exe)";
	}

	// ensure proper variables are set for the specified ASSIGNEE and ACTION_TYPE

	if (assignee == ASSIGNEE::UNIT_TYPE || assignee == ASSIGNEE::UNIT_TYPE_NEAR_LOCATION) {
		assert(unit_type != sc2::UNIT_TYPEID::INVALID);
	}

	if (assignee == ASSIGNEE::MATCH_FLAGS || assignee == ASSIGNEE::MATCH_FLAGS_NEAR_LOCATION) {
		assert(flags.size() > 0);
	}

	if (assignee == ASSIGNEE::UNIT_TYPE_NEAR_LOCATION || assignee == ASSIGNEE::MATCH_FLAGS_NEAR_LOCATION) {
		assert(assignee_location != INVALID_POINT);

		// when a point is dynamically specified, it will provide NO_POINT_FOUND
		// in cases a function could not produce a valid point
		if (assignee_location == NO_POINT_FOUND) {
			return false;
		}
	}

	if (action_type == ACTION_TYPE::EXACT_LOCATION || action_type == ACTION_TYPE::NEAR_LOCATION ||
		action_type == ACTION_TYPE::GET_GAS_NEAR_LOCATION || action_type == ACTION_TYPE::GET_MINERALS_NEAR_LOCATION ||
		action_type == ACTION_TYPE::TARGET_UNIT_NEAR_LOCATION) {
		assert(target_location != INVALID_POINT);

		// when a point is dynamically specified, it will provide NO_POINT_FOUND
		// in cases a function could not produce a valid point
		if (target_location == NO_POINT_FOUND) {
			return false;
		}

	}

	if (assignee == ASSIGNEE::UNIT_TYPE || assignee == ASSIGNEE::UNIT_TYPE_NEAR_LOCATION) {
		if (action_type == ACTION_TYPE::EXACT_LOCATION || action_type == ACTION_TYPE::NEAR_LOCATION ||
			action_type == ACTION_TYPE::GET_MINERALS_NEAR_LOCATION) {
			
			if (ability == sc2::ABILITY_ID::BUILD_ASSIMILATOR ||
				ability == sc2::ABILITY_ID::BUILD_EXTRACTOR ||
				ability == sc2::ABILITY_ID::BUILD_REFINERY) {
				// find a geyser near target location to build the structure on
				return executeBuildGasStructure(agent);
			}

			if (ability == sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST) {
				return execute_protoss_nexus_chronoboost(agent);
			}

			return executeOrderForUnitTypeWithLocation(agent);
		}
		if (action_type == SIMPLE_ACTION || action_type == DISABLE_DEFAULT_DIRECTIVE) {
			return executeSimpleActionForUnitType(agent);
		}
	}

	if (assignee == MATCH_FLAGS || assignee == MATCH_FLAGS_NEAR_LOCATION) {
		return executeMatchFlags(agent);
	}
	
	return false;
}

bool Directive::executeForMob(BasicSc2Bot* agent, Mob* mob_) {
	// used to assign an order to a specific unit
	Mob* mob = &agent->mobH->getMob(mob_->unit);
	if (!mob) {
		return false;
	}

	sc2::Point2D location = target_location;

	if (location == ASSIGNED_LOCATION) {
		location = mob->getAssignedLocation();
	}

	if (update_assignee_location)
		updateAssigneeLocation(agent);

	if (update_target_location) {
		updateTargetLocation(agent);
	}
		

	if (action_type == GET_MINERALS_NEAR_LOCATION) {

		// Note: this order cannot have further queued directives attached,
		//       as harvesting continues indefinitely
		
		
		//if (mob->is_carrying_minerals()) {
			// if unit is carrying minerals, return them to the townhall instead
		//	const sc2::Unit* townhall = agent->locH->getNearestTownhall(location);
		//	if (!townhall)
		//		return false;

			/* ORDER IS EXECUTED */
		//	return issueOrder(agent, mob, false, sc2::ABILITY_ID::HARVEST_RETURN);
			/* * * * * * * * * * */
		//}
	

		const sc2::Unit* mineral_target = agent->locH->getNearestMineralPatch(location);

		if (!mineral_target) {
			return false;
		}

		/* ORDER IS EXECUTED */
		return issueOrder(agent, mob, mineral_target, false, sc2::ABILITY_ID::HARVEST_GATHER);
		//agent->Actions()->UnitCommand(&mob->unit, ability, mineral_target);
		/* * * * * * * * * * */
	}
	if (action_type == GET_GAS_NEAR_LOCATION) {

		// Note: this order cannot have further queued directives attached
		//       as harvesting continues indefinitely
		
		if (mob->isCarryingGas()) {
			// if unit is carrying gas, return them to the townhall instead
			const sc2::Unit* townhall = agent->locH->getNearestTownhall(location);
			if (!townhall)
				return false;

			/* ORDER IS EXECUTED */
			return issueOrder(agent, mob, false, sc2::ABILITY_ID::HARVEST_RETURN);
			/* * * * * * * * * * */
		}

		const sc2::Unit* gas_target = agent->locH->getNearestGasStructure(location);
		if (!gas_target) {
			return false;
		}
		/* ORDER IS EXECUTED */
		return issueOrder(agent, mob, false, sc2::ABILITY_ID::SMART);
		/* * * * * * * * * * */
	}
	if (action_type == SIMPLE_ACTION) {

		/* ORDER IS EXECUTED */
		return issueOrder(agent, mob);
		/* * * * * * * * * * */
	}
	if (action_type == EXACT_LOCATION || action_type == NEAR_LOCATION) {
		sc2::Point2D loc = location;

		if (action_type == NEAR_LOCATION)
			loc = uniform_random_point_in_circle(location, proximity);

		/* ORDER IS EXECUTED */
		return issueOrder(agent, mob, loc);
		/* * * * * * * * * * */
	}
	return false;
}

bool Directive::executeSimpleActionForUnitType(BasicSc2Bot* agent) {
	// perform an action that does not require a target unit or point

	std::unordered_set<Mob*> mobs = agent->mobH->getMobs(); // unordered_set of all friendly units
	Mob* mob;


	// filter idle units which match unit_type
	mobs = filterByUnitType(mobs, unit_type);
	mobs = filterIdle(mobs);

	if (assignee == UNIT_TYPE_NEAR_LOCATION) {
		assert(assignee_location != INVALID_POINT);
		assert(assignee_proximity != INVALID_RADIUS);
		mobs = filterNearLocation(mobs, assignee_location, assignee_proximity);
	}

	if (mobs.size() == 0)
		return false;

	mob = getRandomMobFromSet(mobs);

	if (action_type == DISABLE_DEFAULT_DIRECTIVE) {
		// since this is only used for proxy workers right now, going to use this flag until more functionality is implemented
		if (!mob->hasFlag(FLAGS::IS_PROXY)) {
			mob->disableDefaultDirective();
			issueOrder(agent, mob, false, sc2::ABILITY_ID::STOP); //override the ability to be STOP
		}
		mob->setFlag(FLAGS::IS_PROXY);
		return true;
	}

	/* ORDER IS EXECUTED */
	return issueOrder(agent, mob);
	/* * * * * * * * * * */
}

bool Directive::executeBuildGasStructure(BasicSc2Bot* agent) {
	// perform the necessary actions to have a gas structure built closest to the specified target_location

	std::unordered_set<Mob*> mobs = agent->mobH->getMobs(); // vector of all friendly units
	Mob* mob; // used to store temporary mob
	bool found_valid_unit = false;

	// find a vespene geyser near the location and build on it
	const sc2::Unit* geyser_target = agent->locH->getNearestGeyser(target_location);
	sc2::Point2D location = geyser_target->pos;

	if (assignee == UNIT_TYPE_NEAR_LOCATION) {
		mobs = filterNearLocation(mobs, assignee_location, assignee_proximity);
	}

	mobs = filterByUnitType(mobs, unit_type);

	// there are no valid units to perform this action
	if (mobs.size() == 0)
		return false;

	// check if any of this unit type are already building a gas structure
	//  if (is_any_executing_order(mobs, ability))
	//	  return false;     *****
	
	// pick valid mob to execute order, closest to geyser
	mob = getClosestToLocation(mobs, target_location);

	if (!mob)
		return false;

	/* ORDER IS EXECUTED */
	mob->setFlag(FLAGS::BUILDING_GAS);  // flag is necessary to prevent protoss probe from staying frozen
	mob->setFlag(FLAGS::IS_BUILDING_STRUCTURE);
	return issueOrder(agent, mob, geyser_target);
	/* * * * * * * * * * */


}

bool Directive::execute_protoss_nexus_chronoboost(BasicSc2Bot* agent) {
	// this is more complex than it originally seemed
	// must find the clostest nexus to the given location
	// that has the chronoboost ability ready
	// then find a structure that would benefit from it

	std::unordered_set<Mob*> mobs = agent->mobH->getMobs(); // vector of all friendly units
	Mob* mob; // used to store temporary mob
	Mob* chrono_target;
	std::unordered_set<Mob*> mobs_filter1;
	static const sc2::Unit* unit_to_target;
	
	
	// allow a strategy to specify a specific target for chronoboost by invoking
	// bot->storeUnitType("_CHRONOBOOST_TARGET", <sc2::UNIT_TYPEID>)
	sc2::UNIT_TYPEID _chronotarget_type = agent->getUnitType("_CHRONOBOOST_TARGET");
	bool _special_chronotarget = (_chronotarget_type != sc2::UNIT_TYPEID::INVALID);

	if (assignee == UNIT_TYPE_NEAR_LOCATION) {
		mobs = filterNearLocation(mobs, assignee_location, assignee_proximity);
	}

	// first get list of Mobs matching type
	mobs = filterByUnitType(mobs, unit_type);

	// then filter by those with ability available
	std::unordered_set<Mob*> mobs_filter;
	std::copy_if(mobs.begin(), mobs.end(), std::inserter(mobs_filter, mobs_filter.begin()),
		[agent, this](Mob* m) { return agent->canUnitUseAbility(m->unit, ability); });
	mobs = mobs_filter;
	
	// return false if no structures exist with chronoboost ready to cast
	int num_units = mobs.size();
	if (num_units == 0)
		return false;

	// then pick the one closest to location
	mob = getClosestToLocation(mobs, target_location);

	// return false if mob has not been assigned above
	if (!mob)
		return false;

	// get all structures
	std::unordered_set<Mob*> structures = agent->mobH->filterByFlag(agent->mobH->getMobs(), FLAGS::IS_STRUCTURE);

	// if a special chronotarget structure was specified in the strategy, filter by that structure
	if (_special_chronotarget) {
		std::unordered_set<Mob*> chrono_structures;
		std::copy_if(structures.begin(), structures.end(), std::inserter(chrono_structures, chrono_structures.begin()),
			[this, _chronotarget_type](Mob* m) { return (m->unit.unit_type == _chronotarget_type); });
		structures = chrono_structures;
	}

	std::unordered_set<Mob*> structures_with_orders; 
	// look for buildings that are doing something
	std::copy_if(structures.begin(), structures.end(), std::inserter(structures_with_orders, structures_with_orders.begin()),
		[this](Mob* m) { return (m->unit.orders).size() > 0; });

	if (structures_with_orders.size() == 0)
		return false;

	std::unordered_set<Mob*> not_already_chronoboosted;
	// make sure the target already isn't chronoboosted

	/*
	bool is_chronoboosted;
	for (auto it = structures_with_orders.begin(); it != structures_with_orders.end(); ++it) {
		std::vector<sc2::BuffID> buffs = (*it)->unit.buffs;
		is_chronoboosted = false;
		for (auto b : buffs) {
			if (b == sc2::BUFF_ID::CHRONOBOOSTENERGYCOST)
				is_chronoboosted = true;
			if (!is_chronoboosted) {
				std::cout << "added something to not_already_chronoboosted" << std::endl;
				not_already_chronoboosted.insert(*it);
			}
		}
	}
	*/

	
	std::copy_if(structures_with_orders.begin(), structures_with_orders.end(), std::inserter(not_already_chronoboosted, not_already_chronoboosted.begin()),
		[this](Mob* m) { return (std::find(m->unit.buffs.begin(), m->unit.buffs.end(), sc2::BUFF_ID::CHRONOBOOSTENERGYCOST) == m->unit.buffs.end()); }); 

	if (not_already_chronoboosted.empty()) {
		//std::cout << "there are no non-chronoboosted structures apparently" << std::endl;
		return false;
	}

	// then pick one of THESE closest to location
	chrono_target = getClosestToLocation(not_already_chronoboosted, target_location);

	// return false if there is nothing worth casting chronoboost on
	if (!chrono_target) {
		return false;
	}

	/* ORDER IS EXECUTED */
	return issueOrder(agent, mob, &chrono_target->unit);
	/* * * * * * * * * * */
}

bool Directive::executeMatchFlags(BasicSc2Bot* agent) {
	// issue an order to units matching the provided flags

	std::unordered_set<Mob*> mobs = agent->mobH->getMobs(); // vector of all friendly units
	std::unordered_set<Mob*> matching_mobs = agent->mobH->filterByFlags(mobs, flags);

	// get only units near the assignee_location parameter
	if (assignee == MATCH_FLAGS_NEAR_LOCATION) {
		matching_mobs = filterNearLocation(matching_mobs, assignee_location, assignee_proximity);
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
			}
		}
	}
	if (!found_valid_unit)
		return false;

	/* ORDER IS EXECUTED */
	return issueOrder(agent, filtered_mobs, location);
	/* * * * * * * * * * */
}

bool Directive::executeOrderForUnitTypeWithLocation(BasicSc2Bot* agent) {
	const sc2::AbilityData ability_data = agent->Observation()->GetAbilityData()[(int)ability]; // various info about the ability
	sc2::QueryInterface* query_interface = agent->Query(); // used to query data
	sc2::Point2D location = target_location;
	std::unordered_set<Mob*> mobs = agent->mobH->getMobs();
	Mob* mob = nullptr;

	if (assignee == UNIT_TYPE_NEAR_LOCATION) {
		mobs = filterNearLocation(mobs, assignee_location, assignee_proximity);
	}

	if (action_type == ACTION_TYPE::NEAR_LOCATION) {
		location = uniform_random_point_in_circle(target_location, proximity);
	}

	mobs = filterByUnitType(mobs, unit_type);

	// check if a unit of this type is already executing this order
	//if (is_any_executing_order(mobs, ability)) {
	//	return false;
	//}   *****

	// unit of type does not exist
	if (mobs.size() == 0) {
		return false;
	}


	mobs = filterByHasAbility(agent, mobs, ability);
	if (mobs.size() == 0) {
		return false;
	}

    
	// if order is a build structure order, ensure a valid placement location
	if (ability_data.is_building) {
		
		// if any unit is currently already on route to build this structure
		if (ifAnyOnRouteToBuild(agent, mobs)) {
			return false;
		}


		int i = 0;
		if (action_type == ACTION_TYPE::NEAR_LOCATION) {
			while (!query_interface->Placement(ability, location)) {
				location = uniform_random_point_in_circle(target_location, proximity);
				i++;
				if (i > 20) {
					// can't find a suitable spot to build
					return false;
				}
			}
		}
		else {
			if (!query_interface->Placement(ability, location)) {
				//std::cout << "%";
				return false;
			}
		}

	// filter only those not currently building a structure
	mobs = filterNotBuildingStructure(agent, mobs);

		if (mobs.size() == 0) {
			return false;
		}
	}

	if (allow_multiple) {
		mobs = filterNotAssignedToThis(mobs);
	}

	if (mobs.size() == 0) {
		return false;
	}

	// get closest matching unit to target location
	mob = getClosestToLocation(mobs, target_location);

	if (!mob) {
		return false;
	}

	if (action_type == GET_MINERALS_NEAR_LOCATION) {

		// Note: this order should not have further queued directives attached,
		//       as harvesting continues indefinitely


		const sc2::Unit* mineral_target = agent->locH->getNearestMineralPatch(target_location);

		if (!mineral_target) {
			return false;
		}

		/* ORDER IS EXECUTED */
		bool ordersuccess = false;
		ordersuccess = issueOrder(agent, mob, mineral_target->pos, false, sc2::ABILITY_ID::GENERAL_MOVE);
		//ordersuccess = issueOrder(agent, mob, mineral_target, false, sc2::ABILITY_ID::SMART);
		if (ordersuccess) {
			mob->setAssignedLocation(target_location);
		}
		return ordersuccess;
		//return issueOrder(agent, mob, mineral_target->pos, false, sc2::ABILITY_ID::GENERAL_MOVE);
		/* * * * * * * * * * */
	}

	// if the ability does not require a target location
	if (ability_data.target == sc2::AbilityData::Target::None) {
		
		/* ORDER IS EXECUTED */
		return issueOrder(agent, mob);
		/* * * * * * * * * * */
	}

	// if the ability requires a target unit
	if (ability_data.target == sc2::AbilityData::Target::Unit) {
		std::cerr << ability_data.friendly_name << " requires a target unit." << std::endl;
		return false;
	}

	// else, the ability requires a target location

	/* ORDER IS EXECUTED */
	bool order_success = false;
	order_success = issueOrder(agent, mob, location);
	if (order_success && ability_data.is_building) {
		mob->setFlag(FLAGS::IS_BUILDING_STRUCTURE);
	}
	return order_success;
	/* * * * * * * * * * */
}

bool Directive::haveBundle() {
	// if this directive has other directives bundled with it
	return (directive_bundle.size() > 0);
}

size_t Directive::getID()
{
	return id;
}

bool Directive::ifAnyOnRouteToBuild(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_) {
	const sc2::ObservationInterface* obs = agent->Observation();

	for (auto it = mobs_.begin(); it != mobs_.end(); ++it) {
		for (const auto& order : (*it)->unit.orders) {
			if (order.ability_id == ability && order.progress == 0) {
				return true;
			}
		}
	}
	return false;
}

bool Directive::isBuildingStructure(BasicSc2Bot* agent, Mob* mob_) {
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

bool Directive::isExecutingOrder(std::unordered_set<Mob*> mobs_set, sc2::ABILITY_ID ability_) {
	// check if any Mob in the vector is already executing the specified order
	for (Mob* m : mobs_set) {
		for (const auto& order : m->unit.orders) {
			if (order.ability_id == ability_)
				return true;
		}
	}
	return false;
}

Mob* Directive::getClosestToLocation(std::unordered_set<Mob*> mobs_set, sc2::Point2D pos_) {
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

sc2::ABILITY_ID Directive::getAbilityID()
{
	return ability;
}

int Directive::getTargetUpdateIterationID() {
	return target_update_iter_id;
}

int Directive::getAssigneeUpdateIterationID() {
	return assignee_update_iter_id;
}

bool Directive::allowMultiple(bool is_true) {
	// allow this directive to be assigned to more than one Mob
	if (!locked) {
		allow_multiple = is_true;
		return true;
	}
	return false;
}

void Directive::setContinuous(bool is_true) {
	// when true, this directive will continuously re-issue orders when locations are updated to new values
	continuous_update = is_true;
}

std::unordered_set<Mob*> Directive::filterNearLocation(std::unordered_set<Mob*> mobs_set, sc2::Point2D pos_, float radius_) {
	// filters a vector of Mob* by only those within the specified distance to location
	float sq_dist = pow(radius_, 2);

	std::unordered_set<Mob*> filtered_mobs;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered_mobs, filtered_mobs.begin()),
		[sq_dist, pos_](Mob* m) { return (
			sc2::DistanceSquared2D(m->unit.pos, pos_) <= sq_dist);
		});
	return filtered_mobs;
}

std::unordered_set<Mob*> Directive::filterNotBuildingStructure(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_set) {
	// returns only units that are not currently constructing a structure
	
	std::unordered_set<Mob*> filtered;

	/*
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered, filtered.begin()),
		[this, agent](Mob* m) { return !is_building_structure(agent, m); });
		*/

	filtered = agent->mobH->filterByFlag(mobs_set, FLAGS::IS_BUILDING_STRUCTURE, false);
	return filtered;

}

std::unordered_set<Mob*> Directive::filterByHasAbility(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_set, sc2::ABILITY_ID ability_) {
	// get only mobs from set that have a specified ability

	std::unordered_set<Mob*> mobs_filter;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(mobs_filter, mobs_filter.begin()),
		[agent, this](Mob* m) { return agent->canUnitUseAbility(m->unit, ability); });

	return mobs_filter;
}

std::unordered_set<Mob*> Directive::filterByUnitType(std::unordered_set<Mob*> mobs_set, sc2::UNIT_TYPEID unit_type_) {
	std::unordered_set<Mob*> filtered;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered, filtered.begin()),
		[this](Mob* m) { return m->unit.unit_type == unit_type; });
	return filtered;
}

std::unordered_set<Mob*> Directive::filterIdle(std::unordered_set<Mob*> mobs_set) {
	std::unordered_set<Mob*> filtered;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered, filtered.begin()),
		[this](Mob* m) { return (m->unit.orders).size() == 0; });
	return filtered;
}

std::unordered_set<Mob*> Directive::filterNotAssignedToThis(std::unordered_set<Mob*> mobs_set) {
	std::unordered_set<Mob*> filtered;
	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered, filtered.begin()),
		[this](Mob* m) { 
			if (!m->hasCurrentDirective())
				return true;
			if (m->getCurrentDirective()->getID() == getID() &&
				(!continuous_update || m->getCurrentDirective()->getTargetUpdateIterationID() == target_update_iter_id)) {
				return false;
			}
			return true;
		});
	return filtered;
}

Mob* Directive::getRandomMobFromSet(std::unordered_set<Mob*> mob_set) {
	int index = rand() % mob_set.size();
	auto it = mob_set.begin();
	for (int i = 0; i < index; ++i)
	{
		++it;
	}
	return *it;
}

void Directive::setDebug(bool is_true) {
	debug = is_true;
}

void Directive::lock() {
	// prevent further modification to this
	locked = true;
}

bool Directive::allowsMultiple() {
	// check if this directive allows multiple mobs to be assigned at once
	return allow_multiple;
}

bool Directive::_genericIssueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, sc2::Point2D target_loc_, const sc2::Unit* target_unit_, bool queued_, sc2::ABILITY_ID ability_) {

	// if no ability override specified
	if (ability_ == USE_DEFINED_ABILITY)
		ability_ = ability;


	sc2::Point2D location = target_loc_;

	bool apply_bundle = haveBundle();
	
	// handle case where more than one mob is being assigned the order
	if (mobs_.size() > 1) {
		
		// check mobs_ for units already assigned this directive
		for (auto iter = mobs_.begin(); iter != mobs_.end(); ) {
			auto next = std::next(iter);
			if ((*iter)->hasCurrentDirective()) {
				Directive* curr_dir = (*iter)->getCurrentDirective();
				// if mob at iter is already assigned to this directive
				if (curr_dir->getID() == getID()) {
					// if continuous_update, erase only if target_location is the same
					if (!continuous_update ||
						continuous_update && curr_dir->getTargetUpdateIterationID() == target_update_iter_id) {
						mobs_.erase(*iter);
					}
				}
			}
			iter = next;
		}

		if (mobs_.empty()) {
			return false;
		}		
		
		sc2::Units units;
		for (auto m_ : mobs_) {
			units.push_back(&(m_->unit));
		}

		bool action_success = false;

		// no target is specified
		if (target_loc_ == INVALID_POINT && target_unit_ == nullptr) {
			agent->Actions()->UnitCommand(units, ability_, queued_);
			action_success = true;
		}

		// target location is specified
		if (target_loc_ != INVALID_POINT && target_unit_ == nullptr) {
			// if point is SEND_HOME, grab the first unit's home location instead
			if (target_loc_ == SEND_HOME) {
				target_loc_ = agent->mobH->getMob(*units.front()).getHomeLocation();
			}
			agent->Actions()->UnitCommand(units, ability_, target_loc_, queued_);
			action_success = true;
		}

		// target unit is specified
		if (target_loc_ == INVALID_POINT && target_unit_ != nullptr) {
			agent->Actions()->UnitCommand(units, ability_, target_unit_, queued_);
			action_success = true;
		}
		if (action_success) {
			if (assignee != DEFAULT_DIRECTIVE) {
				for (auto m_ : mobs_) {
					assignMob(m_);
					agent->mobH->setMobBusy(m_);
					if (apply_bundle) {
						m_->bundleDirectives(directive_bundle);
						apply_bundle = false;  // bundle is only given to one mob
					}
				}
			}
			else {
				for (auto iter = mobs_.begin(); iter != mobs_.end(); ) {
					auto next = std::next(iter);
					agent->mobH->setMobIdle((*iter), false);
					iter = next;
				}
			}
		}
	} 
	else {
		// handle case where only one mob is being assigned the order

		if (ability == TWI) {
			std::cout << "(1)";
		}

		// this order already has a mob
		if (!allow_multiple && hasAssignedMob()) {
			return false;
		}



		Mob* mob_ = &agent->mobH->getMob((*mobs_.begin())->unit);
		bool action_success = false;
		
		// no target is specified
		if (target_loc_ == INVALID_POINT && target_unit_ == nullptr) {
			agent->Actions()->UnitCommand(&mob_->unit, ability_, queued_);
			action_success = true;
		}

		
		// target location is specified
		if (target_loc_ != INVALID_POINT && target_unit_ == nullptr) {
			if (target_loc_ == SEND_HOME) {
				target_loc_ = mob_->getHomeLocation();
			}
			agent->Actions()->UnitCommand(&mob_->unit, ability_, target_loc_, queued_);
			action_success = true;
		}

		// target unit is specified
		if (target_loc_ == INVALID_POINT && target_unit_ != nullptr) {
			agent->Actions()->UnitCommand(&mob_->unit, ability_, target_unit_, queued_);
			action_success = true;
		}

		if (action_success) {
			if (ability == TWI) {
				std::cout << "(2)";
			}
			if (assignee != DEFAULT_DIRECTIVE) {
				Directive* last_directive = mob_->getCurrentDirective();

				// free up the previous directive from this mob
				if (last_directive != nullptr) {
					last_directive->unassignMob(mob_);
				}
				else {
				}

				assignMob(mob_);

				agent->mobH->setMobBusy(mob_);
				mob_->assignDirective(this);

				if (apply_bundle)
					mob_->bundleDirectives(directive_bundle);

				return true;
			}
			else {
				Directive* last_directive = mob_->getCurrentDirective();
				if (last_directive != nullptr) {
					last_directive->unassignMob(mob_);
				}
				agent->mobH->setMobIdle(mob_, false);
				return true;
			}
		}
	}
	
	return false;
}

bool Directive::issueOrder(BasicSc2Bot* agent, Mob* mob_, bool queued_, sc2::ABILITY_ID ability_) {
	return _genericIssueOrder(agent, std::unordered_set<Mob*>{ mob_ }, INVALID_POINT, nullptr, queued_, ability_);
}

bool Directive::issueOrder(BasicSc2Bot* agent, Mob* mob_, sc2::Point2D target_loc_, bool queued_, sc2::ABILITY_ID ability_) {
	return _genericIssueOrder(agent, std::unordered_set<Mob*>{ mob_ }, target_loc_, nullptr, queued_, ability_);
}

bool Directive::issueOrder(BasicSc2Bot* agent, Mob* mob_, const sc2::Unit* target_unit_, bool queued_, sc2::ABILITY_ID ability_) {
	return _genericIssueOrder(agent, std::unordered_set<Mob*>{ mob_ }, INVALID_POINT, target_unit_, queued_, ability_);
}

bool Directive::issueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, bool queued_, sc2::ABILITY_ID ability_) {
	return _genericIssueOrder(agent, mobs_, INVALID_POINT, nullptr, queued_, ability_);
}

bool Directive::issueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, sc2::Point2D target_loc_, bool queued_, sc2::ABILITY_ID ability_) {
	return _genericIssueOrder(agent, mobs_, target_loc_, nullptr, queued_, ability_);
}

bool Directive::issueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, const sc2::Unit* target_unit_, bool queued_, sc2::ABILITY_ID ability_) {
	return _genericIssueOrder(agent, mobs_, INVALID_POINT, target_unit_, queued_, ability_);
}

bool Directive::setDefault() {
	// a default directive is something that a unit performs when it has no actions
	// usually used for workers to return to gathering after building/defending
	if (!locked) {
		assignee = DEFAULT_DIRECTIVE;
		allow_multiple = true;
	}
	return !locked;
}

std::unordered_set<Mob*> Directive::getAssignedMobs() {
	return assigned_mobs;
}

bool Directive::hasAssignedMob() {
	return !assigned_mobs.empty();
}

bool Directive::assignMob(Mob* mob_) {
	// adds a mob to this directive's assigned mobs
	// returns false if directive does not allow multiple mobs and already has one
	if (!allow_multiple && !assigned_mobs.empty()) {
		return false;
	}

	assigned_mobs.insert(mob_);
	return true;
}

void Directive::unassignMob(Mob* mob_) {
	// unassigns the mob from list of assigned mobs
	
	//std::cout << " directive ability type: " << (int) ability << "directive id: " << getID() << std::endl;
	if (!assigned_mobs.empty()) {
		assigned_mobs.erase(mob_);
	}
	
}

//void Directive::setTargetLocationFunction(std::function<sc2::Point2D(void)> function_) {
void Directive::setTargetLocationFunction(Strategy* strat_, BasicSc2Bot* agent_, std::function<sc2::Point2D ()> function_) {
	strategy_ref = strat_;
	update_target_location = true;
	target_location_function = function_;
	
}

void Directive::setAssigneeLocationFunction(BasicSc2Bot* agent_, std::function<sc2::Point2D()> function_) {
	update_assignee_location = true;
	BasicSc2Bot* bot = agent_;
	assignee_location_function = function_;
}

void Directive::updateAssigneeLocation(BasicSc2Bot* agent_) {
	static int a_iter_id = 0;
	sc2::Point2D prev_location = assignee_location;
	assignee_location = assignee_location_function();
	if (prev_location != assignee_location) {
		assignee_update_iter_id = a_iter_id++;
	}
}

void Directive::updateTargetLocation(BasicSc2Bot* agent_) {
	static int t_iter_id = 0;
	sc2::Point2D prev_location = target_location;
	target_location = target_location_function();
	//std::cout << "target location updated to be (" << target_location.x << ", " << target_location.y << ")" << std::endl;
	if (target_location == NO_POINT_FOUND) {
		target_location == SEND_HOME;
	}
	if (prev_location != target_location) {
		target_update_iter_id = t_iter_id++;
	}
}

sc2::Point2D Directive::uniform_random_point_in_circle(sc2::Point2D center, float radius) {
	// given the center point and radius, return a uniform random point within the circle

	float r = radius * sqrt(sc2::GetRandomScalar());
	float theta = sc2::GetRandomScalar() * 2 * M_PI;
	float x = center.x + r * cos(theta);
	float y = center.y + r * sin(theta);
	return sc2::Point2D(x, y);
}

void Directive::setPriority(int priority) {
	this->priority = priority;
}
int Directive::getPriority(){
	return this->priority;
}