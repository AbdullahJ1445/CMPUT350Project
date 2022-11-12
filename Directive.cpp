#include <cassert>
#include "sc2api/sc2_api.h"
#include "Directive.h"
#include "Agents.h"


Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_) {
	// This constructor is only valid when a unit type is targeting a Point2D location
	assert(assignee_ == ASSIGNEE::UNIT_TYPE);
	assert(action_type_ == ACTION_TYPE::EXACT_LOCATION ||
		action_type_ == ACTION_TYPE::NEAR_LOCATION ||
		action_type_ == ACTION_TYPE::GET_MINERALS_NEAR_LOCATION ||
		action_type_ == ACTION_TYPE::GET_GAS_NEAR_LOCATION);
	action_type = action_type_;
	assignee = assignee_;
	unit_type = unit_type_;
	ability = ability_;
	target_location = location_;
}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Unit target_) {
	// This constructor is only valid when a unit type is targeting a unit
	assert(assignee_ == ASSIGNEE::UNIT_TYPE);
	assert(action_type_ == ACTION_TYPE::TARGET_UNIT);
	action_type = action_type_;
	assignee = assignee_;
	unit_type = unit_type_;
	ability = ability_;
	target_unit = target_;
}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::Point2D location_) {
	// This constructor is only valid for gathering minerals/gas as a default directive
	assert(assignee_ == ASSIGNEE::DEFAULT_DIRECTIVE);
	assert(action_type_ == ACTION_TYPE::GET_MINERALS_NEAR_LOCATION ||
		action_type_ == ACTION_TYPE::GET_GAS_NEAR_LOCATION);
	action_type = action_type_;
	assignee = assignee_;
	target_location = location_;
}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_) {
	// This constructor is only valid for simple actions for a unit_type
	assert(action_type_ == ACTION_TYPE::SIMPLE_ACTION);
	assert(assignee_ == ASSIGNEE::UNIT_TYPE);
	assignee = assignee_;
	unit_type = unit_type_;
	action_type = action_type_;
	ability = ability_;
}

// CC
Directive::Directive(const Directive& rhs) {
	assignee = rhs.assignee;
	action_type = rhs.action_type;
	unit_type = rhs.unit_type;
	ability = rhs.ability;
	target_location = rhs.target_location;
	target_unit = rhs.target_unit;
}

// todo: implement constructors involving squads

bool Directive::execute(BotAgent* agent, const sc2::ObservationInterface* obs) {
	bool found_valid_unit = false; // ensure unit has been assigned before issuing order

	if (assignee == ASSIGNEE::UNIT_TYPE) {
		if (action_type == ACTION_TYPE::EXACT_LOCATION || action_type == ACTION_TYPE::NEAR_LOCATION) {
			if (ability == sc2::ABILITY_ID::BUILD_ASSIMILATOR ||
				ability == sc2::ABILITY_ID::BUILD_EXTRACTOR ||
				ability == sc2::ABILITY_ID::BUILD_REFINERY) {
				
				// find a vespene geyser near the location and build on it

				const sc2::Unit* geyser_target = agent->FindNearestGeyser(target_location);

				static const sc2::Unit* unit;
				sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
				sc2::Point2D location = geyser_target->pos;

				// pick valid unit to execute order, closest to geyser
				float lowest_distance = 99999.0f;
				for (const sc2::Unit* u : units) {
					for (const auto& order : u->orders) {
						if (order.ability_id == ability) {
							// a unit of this type is already executing this order
							return false;
						}
					}
					if (u->unit_type == unit_type) {
						float dist = sc2::DistanceSquared2D(u->pos, location);
						if (dist < lowest_distance) {
							unit = u;
							lowest_distance = dist;
							found_valid_unit = true;
						}
					}
				}
				if (found_valid_unit) {
					agent->Actions()->UnitCommand(unit, ability, geyser_target);
					return true;
				}
				return false;
			}
			static const sc2::Unit* unit;
			sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
			sc2::Point2D location = target_location;

			if (action_type == ACTION_TYPE::NEAR_LOCATION) {
				// add random offsets when NEAR_LOCATION order is specified
				float rx = sc2::GetRandomScalar();
				float ry = sc2::GetRandomScalar();
				location = sc2::Point2D(target_location.x + rx * 7.5f, target_location.y + ry * 10.0f);
			}
			
			// pick valid unit to execute order, closest to target location
			float lowest_distance = 99999.0f;
			for (const sc2::Unit* u : units) {
				for (const auto& order : u->orders) {
					if (order.ability_id == ability) {
						// a unit of this type is already executing this order
						return false;
					}
				}
				if (u->unit_type == unit_type) {
					float dist = sc2::DistanceSquared2D(u->pos, location);
					if (dist < lowest_distance) {
						unit = u;
						lowest_distance = dist;
						found_valid_unit = true;
					}
				}
			}
			if (found_valid_unit) {
				agent->Actions()->UnitCommand(unit, ability, location);
			}
			return true;
		}
		if (action_type == SIMPLE_ACTION) {
			static const sc2::Unit* unit = nullptr;
			sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
			for (const sc2::Unit* u : units) {
				int order_count = (u->orders).size();
				if (u->unit_type == unit_type && order_count == 0) {
					unit = u;
					found_valid_unit = true;
					break;
				}
			}
			if (found_valid_unit) {
				agent->Actions()->UnitCommand(unit, ability);
			};
		}
	}
	return false;
}

bool Directive::executeForUnit(BotAgent* agent, const sc2::ObservationInterface* obs, const sc2::Unit& unit) {
	// used to assign an order to a specific unit

	if (action_type == GET_MINERALS_NEAR_LOCATION) {
		const sc2::Unit * mineral_target = agent->FindNearestMineralPatch(target_location);
		if (!mineral_target) {
			return false;
		}
		agent->Actions()->UnitCommand(&unit, sc2::ABILITY_ID::SMART, mineral_target);
		return true;
	}
	if (action_type == GET_GAS_NEAR_LOCATION) {
		const sc2::Unit* gas_target = agent->FindNearestGasStructure(target_location);
		if (!gas_target) {
			return false;
		}
		agent->Actions()->UnitCommand(&unit, sc2::ABILITY_ID::HARVEST_GATHER, gas_target);
		return true;
	}
	return false;
}

void Directive::setDefault() {
	assignee = DEFAULT_DIRECTIVE;
}