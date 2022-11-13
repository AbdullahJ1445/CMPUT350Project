#include <cassert>
#include "sc2api/sc2_api.h"
#include "Directive.h"
#include "Agents.h"


Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_) {
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
	proximity = proximity_;
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

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::Point2D location_, float proximity_) {
	// This constructor is only valid for gathering minerals/gas as a default directive
	assert(assignee_ == ASSIGNEE::DEFAULT_DIRECTIVE);
	assert(action_type_ == ACTION_TYPE::GET_MINERALS_NEAR_LOCATION ||
		action_type_ == ACTION_TYPE::GET_GAS_NEAR_LOCATION);
	action_type = action_type_;
	assignee = assignee_;
	target_location = location_;
	proximity = proximity_;
}

Directive::Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_) {
	// This constructor is only valid for issuing an order to all units matching a flag, towards a location
	assignee = assignee_;
	action_type = action_type_;
	flags = flags_;
	ability = ability_;
	target_location = location_;
	proximity = proximity_;

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
	proximity = rhs.proximity;
}

bool Directive::execute(BotAgent* agent, const sc2::ObservationInterface* obs) {
	bool found_valid_unit = false; // ensure unit has been assigned before issuing order
	const sc2::AbilityData ability_data = obs->GetAbilityData()[(int)ability]; // various info about the ability
	sc2::QueryInterface* query_interface = agent->Query(); // used to query data

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

			if (ability == sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST) {
				// this is more complex than it originally seemed
				// must find the clostest nexus to the given location
				// that has the chronoboost ability ready
				// then find a structure that would benefit from it

				static const sc2::Unit* unit_to_target;
				// first get list of units matching type
				std::vector<const sc2::Unit*> units_matching_type;
				std::copy_if(units.begin(), units.end(), std::back_inserter(units_matching_type),
					[this](const sc2::Unit* u) { return u->unit_type == unit_type; });

				// then filter by units with ability available
				std::vector<const sc2::Unit*> filtered_units;
				std::copy_if(units_matching_type.begin(), units_matching_type.end(), std::back_inserter(filtered_units),
					[agent, this](const sc2::Unit* u) { return agent->AbilityAvailable(*u, ability); });
				int num_units = filtered_units.size();
				if (num_units == 0)
					return false;

				// then pick the one closest to location
				float lowest_distance = 99999.0f;
				bool found_suitable_caster = false;
				for (const sc2::Unit* u : filtered_units) {
					float dist = sc2::DistanceSquared2D(u->pos, location);
					if (dist < lowest_distance) {
						unit = u;
						lowest_distance = dist;
						found_suitable_caster = true;
					}
				}
				if (!found_suitable_caster)
					return false;

				// get all structures
				std::vector<SquadMember*> squads = agent->get_squad_members();
				std::vector<SquadMember*> structures = agent->filter_by_flag(squads, FLAGS::IS_STRUCTURE);
				std::vector<SquadMember*> with_orders; // look for buildings that are doing something

				std::copy_if(structures.begin(), structures.end(), std::back_inserter(with_orders),
					[this](SquadMember* s) { return (s->unit.orders).size() > 0; });

				if (with_orders.size() == 0)
					return false;

				std::cout << "made it here 3" << std::endl;

				// then pick one of THESE closest to location
				lowest_distance = 99999.0f;
				bool found_suitable_target = false;
				for (SquadMember* s : with_orders) {
					float dist = sc2::DistanceSquared2D(s->unit.pos, location);
					if (dist < lowest_distance) {
						unit_to_target = &s->unit;
						lowest_distance = dist;
						found_suitable_target = true;
					}
				}
				if (!found_suitable_target)
					return false;

				agent->Actions()->UnitCommand(unit, ability, unit_to_target);
				return true;
			}

			if (action_type == ACTION_TYPE::NEAR_LOCATION) {
				location = uniform_random_point_in_circle(target_location, proximity);
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
				if (ability_data.is_building) {
					int i = 0;
					while (!query_interface->Placement(ability, location)) {
						location = uniform_random_point_in_circle(target_location, proximity);
						i++;
						if (i > 20) {
							// can't find a suitable spot to build
							//std::cerr << ability_data.friendly_name << " - cannot find suitable building location" << std::endl;
							return false;
						}
					}
				}

				if (ability_data.target == sc2::AbilityData::Target::None) {
					agent->Actions()->UnitCommand(unit, ability);
					return true;
				}
				if (ability_data.target == sc2::AbilityData::Target::Unit) {
					std::cerr << ability_data.friendly_name << " requires a target unit." << std::endl;
					return false;
				}
				agent->Actions()->UnitCommand(unit, ability, location);
				return true;
			}
			return false;
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

	if (assignee == MATCH_FLAGS) {
		std::vector<SquadMember*> squads = agent->get_squad_members();
		std::vector<SquadMember*> matching_squads = agent->filter_by_flags(squads, flags);
		sc2::Point2D location = target_location;
		static const sc2::Unit* unit;
		sc2::Units units;
		if (action_type == ACTION_TYPE::EXACT_LOCATION || action_type == ACTION_TYPE::NEAR_LOCATION) {
			for (SquadMember* s : matching_squads) {
				unit = &(s->unit);
				if (action_type == ACTION_TYPE::NEAR_LOCATION) {
					location = uniform_random_point_in_circle(target_location, proximity);
				}
				// Unit has no orders
				if ((unit->orders).size() == 0) {
					found_valid_unit = true;
					units.push_back(unit);
				}
			}
		}
		if (found_valid_unit) {
			agent->Actions()->UnitCommand(units, ability, location);
		}
		return found_valid_unit;
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
	// a default directive is something that a unit performs when it has no actions
	// usually used for workers to return to gathering after building/defending
	assignee = DEFAULT_DIRECTIVE;
}

sc2::Point2D Directive::uniform_random_point_in_circle(sc2::Point2D center, float radius) {
	// given the center point and radius, return a uniform random point within the circle

	float r = radius * sqrt(sc2::GetRandomScalar());
	float theta = sc2::GetRandomScalar() * 2 * M_PI;
	float x = center.x + r * cos(theta);
	float y = center.y + r * sin(theta);
	return sc2::Point2D(x, y);
}