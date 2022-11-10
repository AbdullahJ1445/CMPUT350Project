#pragma once

#include "Triggers.h"
#include "sc2api/sc2_api.h"

TriggerCondition::TriggerCondition(COND cond_type_, int cond_value_) {
		cond_type = cond_type_;
		cond_value = cond_value_;
	}

bool TriggerCondition::is_met(const sc2::ObservationInterface* obs) {
	switch (cond_type) {
	case MIN_MINERALS:
		return obs->GetMinerals() >= cond_value;
	case MIN_GAS:
		return obs->GetVespene() >= cond_value;
	case MIN_TIME:
		return obs->GetGameLoop() >= cond_value;
	case MIN_FOOD:
		return obs->GetFoodUsed() >= cond_value;
	case MIN_FOOD_CAP:
		return obs->GetFoodCap() >= cond_value;
	case MAX_MINERALS:
		return obs->GetMinerals() <= cond_value;
	case MAX_GAS:
		return obs->GetVespene() <= cond_value;
	case MAX_TIME:
		break;
	case MAX_FOOD:
		break;
	case MAX_FOOD_CAP:
		break;
	}
	return false;
}

Trigger::Trigger() {};

void Trigger::add_condition(TriggerCondition tc_) {
	conditions.push_back(tc_);
}
bool Trigger::check_conditions(const sc2::ObservationInterface* obs) {
	// Iterate through all conditions and return false if any are not met.
	// Otherwise return true.
	for (auto c_ : conditions) {
		if (!c_.is_met(obs))
			return false;
	}
	return true;
}


StrategyOrder::StrategyOrder(Bot* agent_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D unit_location_, sc2::Point2D target_location_) {
	agent = agent_;
	unit_type = unit_type_;
	ability = ability_;
	unit_location = unit_location_;
	target_location = target_location_;
}

bool StrategyOrder::execute(const sc2::ObservationInterface* obs) {
	static const sc2::Unit* unit;
	sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);

	// pick valid unit to execute order
	for (const sc2::Unit* u : units) {
		float lowest_distance = 99999.0f;
		for (const auto& order : u->orders) {
			if (order.ability_id == ability) {
				return false;
			}
		}
		if (u->unit_type == unit_type) {
			float dist = sc2::DistanceSquared2D(u->pos, unit_location);
			if (dist < lowest_distance) {
				unit = u;
				lowest_distance = dist;
			}
		}
	}
	std::cout << "attempting to build pylon at " << target_location.x << "," << target_location.y << std::endl;
	agent->Actions()->UnitCommand(unit, ability, target_location);
	return true;
}

void StrategyOrder::addTriggerCondition(COND cond_type_, int cond_value_) {
	TriggerCondition tc(cond_type_, cond_value_);
	trigger.add_condition(tc);
}

bool StrategyOrder::checkTriggerConditions(const sc2::ObservationInterface* obs) {
	return trigger.check_conditions(obs);
}

Trigger StrategyOrder::getTrigger() {
	return trigger;
}