#pragma once

#include "Directive.h"
#include "Triggers.h"
#include "sc2api/sc2_api.h"

TriggerCondition::TriggerCondition(COND cond_type_, int cond_value_) {
		cond_type = cond_type_;
		cond_value = cond_value_;
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
		break;
	case COND::MAX_FOOD:
		break;
	case COND::MAX_FOOD_CAP:
		break;
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
	has_directive = false;
}

bool StrategyOrder::execute(const sc2::ObservationInterface* obs) {
	if (has_directive)
		return directive->execute(agent, obs);
}

void StrategyOrder::setDirective(Directive directive_) {
	if (has_directive)
		delete directive;
	directive = new Directive(directive_);
	has_directive = true;
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