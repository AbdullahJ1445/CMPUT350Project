#pragma once

#include "Directive.h"
#include "Triggers.h"
#include "sc2api/sc2_api.h"

TriggerCondition::TriggerCondition(COND cond_type_, int cond_value_) {
	cond_type = cond_type_;
	cond_value = cond_value_;
}

TriggerCondition::TriggerCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_) {
	assert(cond_type_ == COND::MAX_UNIT_OF_TYPE || cond_type_ == COND::MIN_UNIT_OF_TYPE);
	cond_type = cond_type_;
	cond_value = cond_value_;
	unit_of_type = unit_of_type_;
}

bool TriggerCondition::is_met(const sc2::ObservationInterface* obs) {
	switch (cond_type) {
	case COND::MIN_MINERALS:
		return obs->GetMinerals() >= cond_value;
	case COND::MIN_GAS:
		return obs->GetVespene() >= cond_value;
	case COND::MIN_TIME:
		return obs->GetGameLoop() >= cond_value;
	case COND::MIN_FOOD:
		return obs->GetFoodUsed() >= cond_value;
	case COND::MIN_FOOD_CAP:
		return obs->GetFoodCap() >= cond_value;
	case COND::MAX_MINERALS:
		return obs->GetMinerals() <= cond_value;
	case COND::MAX_GAS:
		return obs->GetVespene() <= cond_value;
	case COND::MAX_TIME:
		return obs->GetGameLoop() <= cond_value;
	case COND::MAX_FOOD:
		return obs->GetFoodCap() - obs->GetFoodUsed() <= cond_value;
	case COND::MAX_FOOD_CAP:
		return obs->GetFoodCap() <= cond_value;
	}
	
	if (cond_type == COND::MAX_UNIT_OF_TYPE || cond_type == COND::MIN_UNIT_OF_TYPE) {
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) { return u->unit_type == unit_of_type; });
		for (auto u : units) {
			
		}
		if (cond_type == COND::MAX_UNIT_OF_TYPE)
			return (num_units <= cond_value);
		if (cond_type == COND::MIN_UNIT_OF_TYPE)
			return (num_units >= cond_value);
	}

	return false;
}

Trigger::Trigger() {
};

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

StrategyOrder::StrategyOrder(BotAgent* agent_) {
	agent = agent_;
}

StrategyOrder::~StrategyOrder() {
	directives.clear();
}

bool StrategyOrder::execute(const sc2::ObservationInterface* obs) {
	return directives.front()->execute(agent, obs);
}

void StrategyOrder::setDirective(Directive directive_) {
	if (directives.size() != 0) {
		directives.clear();
	}
	Directive* directive = new Directive(directive_);
	directives.push_back(directive);
}

void StrategyOrder::addDirective(Directive directive_) {
	// no reason to use this at the moment
	// a trigger will only execute the front directive in the vector
	Directive* directive = new Directive(directive_);
	directives.push_back(directive);
}

void StrategyOrder::addTriggerCondition(COND cond_type_, int cond_value_) {
	TriggerCondition tc(cond_type_, cond_value_);
	trigger.add_condition(tc);
}

void StrategyOrder::addTriggerCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_) {
	TriggerCondition tc(cond_type_, cond_value_, unit_of_type_);
	trigger.add_condition(tc);
}

bool StrategyOrder::checkTriggerConditions(const sc2::ObservationInterface* obs) {
	return trigger.check_conditions(obs);
}

Trigger StrategyOrder::getTrigger() {
	return trigger;
}