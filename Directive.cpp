#include <cassert>
#include "sc2api/sc2_api.h"
#include "Directive.h"
#include "Agents.h"

Directive::Directive(DIR_TYPE dir_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_) {
	// This constructor is only valid when a unit type is executing a simple action
	assert(dir_type_ == DIR_TYPE::UNIT_TYPE_SIMPLE_ACTION);
	dir_type = dir_type_;
	unit_type = unit_type_;
	ability = ability_;
}

Directive::Directive(DIR_TYPE dir_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_) {
	// This constructor is only valid when a unit type is targeting a Point2D location
	assert(dir_type_ == DIR_TYPE::UNIT_TYPE_TO_EXACT_LOCATION ||
		dir_type_ == DIR_TYPE::UNIT_TYPE_TO_NEAR_LOCATION);
	dir_type = dir_type_;
	unit_type = unit_type_;
	ability = ability_;
	target_location = location_;
}

Directive::Directive(DIR_TYPE dir_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Unit target_) {
	assert(dir_type_ == DIR_TYPE::UNIT_TYPE_TO_TARGET);
	dir_type = dir_type_;
	unit_type = unit_type_;
	ability = ability_;
	target_unit = target_;
}

// todo: implement constructors involving squads

bool Directive::execute(BotAgent* agent, const sc2::ObservationInterface* obs) {
	if (dir_type == DIR_TYPE::UNIT_TYPE_TO_EXACT_LOCATION) {
		static const sc2::Unit* unit;
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);

		// pick valid unit to execute order, closest to target location
		for (const sc2::Unit* u : units) {
			float lowest_distance = 99999.0f;
			for (const auto& order : u->orders) {
				if (order.ability_id == ability) {
					// a unit of this type is already executing this order
					return false;
				}
			}
			if (u->unit_type == unit_type) {
				float dist = sc2::DistanceSquared2D(u->pos, target_location);
				if (dist < lowest_distance) {
					unit = u;
					lowest_distance = dist;
				}
			}
		}
		agent->Actions()->UnitCommand(unit, ability, target_location);
	}
	if (dir_type == DIR_TYPE::UNIT_TYPE_TO_NEAR_LOCATION) {
		static const sc2::Unit* unit;
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		sc2::Point2D near_location(-1, -1);

		// pick valid unit to execute order, closest to target location
		float rx = sc2::GetRandomScalar();
		float ry = sc2::GetRandomScalar();
		near_location = sc2::Point2D(target_location.x + rx * 7.5f, target_location.y + ry * 7.5f);
		float lowest_distance = 99999.0f;
		for (const sc2::Unit* u : units) {
			for (const auto& order : u->orders) {
				if (order.ability_id == ability) {
					// a unit of this type is already executing this order
					return false;
				}
			}
			if (u->unit_type == unit_type) {
				float dist = sc2::DistanceSquared2D(u->pos, near_location);
				if (dist < lowest_distance) {
					unit = u;
					lowest_distance = dist;
				}
			}
		}
		agent->Actions()->UnitCommand(unit, ability, near_location);
	}
}