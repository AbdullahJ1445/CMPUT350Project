#pragma once

#include "Directive.h"
#include "Triggers.h"
#include "sc2api/sc2_api.h"

Trigger::TriggerCondition::TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_) {
	cond_type = cond_type_;
	cond_value = cond_value_;
	agent = agent_;
}

Trigger::TriggerCondition::TriggerCondition(BotAgent* agent_, COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_) {
	assert(cond_type_ == COND::HAS_ABILITY_READY);
	cond_type = cond_type_;
	ability_id = ability_id_;
	is_true = is_true_;
	agent = agent_;
	unit_of_type = unit_of_type_;
}

Trigger::TriggerCondition::TriggerCondition(BotAgent* agent_, COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_) {
	assert(cond_type_ == COND::HAVE_UPGRADE);
	cond_type = cond_type_;
	upgrade_id = upgrade_id_;
	is_true = is_true_;
	agent = agent_;
}

Trigger::TriggerCondition::TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_) {
	assert(cond_type_ == COND::MIN_UNIT_WITH_FLAGS ||
		cond_type == COND::MAX_UNIT_WITH_FLAGS);
	cond_type = cond_type_;
	cond_value = cond_value_;
	filter_flags = flags_;
	agent = agent_;
}

Trigger::TriggerCondition::TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_, sc2::Point2D location_, float radius_) {
	assert(cond_type_ == COND::MIN_UNIT_WITH_FLAGS_NEAR_LOCATION || 
		cond_type == COND::MAX_UNIT_WITH_FLAGS_NEAR_LOCATION);
	cond_type = cond_type_;
	cond_value = cond_value_;
	filter_flags = flags_;
	agent = agent_;
	distance_squared = pow(radius_, 2);
	location_for_counting_units = location_;
}

Trigger::TriggerCondition::TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_) {
	assert(cond_type_ == COND::MAX_UNIT_OF_TYPE || cond_type_ == COND::MIN_UNIT_OF_TYPE || cond_type_ == COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION);
	cond_type = cond_type_;
	cond_value = cond_value_;
	unit_of_type = unit_of_type_;
	agent = agent_;
}

Trigger::TriggerCondition::TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_) {
	assert(cond_type_ == COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION || cond_type_ == COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION);
	cond_type = cond_type_;
	cond_value = cond_value_;
	unit_of_type = unit_of_type_;
	location_for_counting_units = location_;
	distance_squared = pow(radius_, 2);
	agent = agent_;
}

bool Trigger::TriggerCondition::is_met(const sc2::ObservationInterface* obs) {
	switch (cond_type) {
	case COND::MIN_MINERALS:
		return obs->GetMinerals() >= cond_value;
	case COND::MIN_GAS:
		return obs->GetVespene() >= cond_value;
	case COND::MIN_TIME:
		return obs->GetGameLoop() >= cond_value;
	case COND::MIN_FOOD:
		return obs->GetFoodCap() - obs->GetFoodUsed() >= cond_value;
	case COND::MIN_FOOD_USED:
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
	case COND::MAX_FOOD_USED:
		return obs->GetFoodUsed() <= cond_value;
	case COND::MAX_FOOD_CAP:
		return obs->GetFoodCap() <= cond_value;
	case COND::MIN_MINERALS_MINED:
		// not implemented
		return false;
	case COND::MAX_MINERALS_MINED:
		// not implemented
		return false;
	case COND::MIN_UNIT_WITH_FLAGS:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->filter_by_flags(agent->get_mobs(), filter_flags);
		int num_units = mobs.size();
		return (num_units >= cond_value);
	}
	case COND::MAX_UNIT_WITH_FLAGS:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->filter_by_flags(agent->get_mobs(), filter_flags);
		int num_units = mobs.size();
		return (num_units <= cond_value);
	}
	case COND::MIN_UNIT_WITH_FLAGS_NEAR_LOCATION:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->filter_by_flags(agent->get_mobs(), filter_flags);
		std::unordered_set<Mob*> filtered_mobs;
		std::copy_if(mobs.begin(), mobs.end(), std::inserter(filtered_mobs, filtered_mobs.begin()),
			[this](Mob* m) { return (
				sc2::DistanceSquared2D(m->unit.pos, location_for_counting_units) <= distance_squared); 
			});
		int num_units = filtered_mobs.size();
		return (num_units >= cond_value);
	}
	case COND::MAX_UNIT_WITH_FLAGS_NEAR_LOCATION:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->filter_by_flags(agent->get_mobs(), filter_flags);
		std::unordered_set<Mob*> filtered_mobs;
		std::copy_if(mobs.begin(), mobs.end(), std::inserter(filtered_mobs, filtered_mobs.begin()),
			[this](Mob* m) { return (
				sc2::DistanceSquared2D(m->unit.pos, location_for_counting_units) <= distance_squared);
			});
		int num_units = filtered_mobs.size();
		return (num_units <= cond_value);
	}
	case COND::BASE_IS_ACTIVE:
		if (agent->bases.size() <= cond_value || cond_value < 0)
			return false;
		return agent->bases[cond_value].is_active();
	case COND::HAVE_UPGRADE:
		return agent->have_upgrade(upgrade_id) == is_true;
	case COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION:
	{
		const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		std::vector<const sc2::Unit*> filtered_units;
		std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
			[this](const sc2::Unit* u) { return (u->unit_type == unit_of_type) && (u->build_progress != 1.0); });
		int num_units = filtered_units.size();
		return (num_units >= cond_value);
	}
	case COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION:
		{
			const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
			std::vector<const sc2::Unit*> filtered_units;
			std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
				[this](const sc2::Unit* u) { return (u->unit_type == unit_of_type) && (u->build_progress != 1.0); });
			int num_units = filtered_units.size();
			return (num_units <= cond_value);
		}
	case COND::HAS_ABILITY_READY:
		std::unordered_set<Mob*> structures = agent->filter_by_flag(agent->get_mobs(), FLAGS::IS_STRUCTURE);
		bool found_one = false;
		for (auto m : structures) {
			if (agent->AbilityAvailable(m->unit, ability_id)) {
				found_one = true;
				break;
			}
		}
		return (found_one == is_true);
	}
	
	if (cond_type == COND::MAX_UNIT_OF_TYPE) {
		// for maximum units we consider units under construction as well, as
		// when it is done it will put the count over the maximum
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) { return u->unit_type == unit_of_type; });
		return (num_units <= cond_value);
	}
	if (cond_type == COND::MIN_UNIT_OF_TYPE) {
		// for minimum units we want to ensure the units have completed construction
		// as this is used to consider whether pre-requisite structures are met
		const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		std::vector<const sc2::Unit*> filtered_units;
		std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
			[this](const sc2::Unit * u) { return (u->unit_type == unit_of_type) && (u->build_progress == 1.0); });
		int num_units = filtered_units.size();
		return (num_units >= cond_value);
	}

	if (cond_type == COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION) {
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) { 
				return (u->unit_type == unit_of_type
					&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared); 
			});
		return (num_units <= cond_value);
	}
	if (cond_type == COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION) {
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) {
				return (u->unit_type == unit_of_type
					&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared) 
					&& (u->build_progress == 1.0);
			});
		return (num_units >= cond_value);
	}
	return false;
}

Trigger::Trigger(BotAgent* agent_) {
	agent = agent_;
};

void Trigger::add_condition(TriggerCondition tc_) {
	conditions.push_back(tc_);
}

void Trigger::add_condition(COND cond_type_, int cond_value_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_);
	conditions.push_back(tc_);
}

void Trigger::add_condition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, unit_of_type_);
	conditions.push_back(tc_);
}

void Trigger::add_condition(COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_) {
	TriggerCondition tc_(agent, cond_type_, unit_of_type_, ability_id_, is_true_);
	conditions.push_back(tc_);
}

void Trigger::add_condition(COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_) {
	TriggerCondition tc_(agent, cond_type_, upgrade_id_, is_true_);
	conditions.push_back(tc_);
}

void Trigger::add_condition(COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, flags_);
	conditions.push_back(tc_);
}

void Trigger::add_condition(COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_, sc2::Point2D location_, float radius_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, flags_, location_, radius_);
	conditions.push_back(tc_);
}

void Trigger::add_condition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, unit_of_type_, location_, radius_);
	conditions.push_back(tc_);
}


bool Trigger::check_conditions() {
	// Iterate through all conditions and return false if any are not met.
	// Otherwise return true.
	const sc2::ObservationInterface* obs = agent->Observation();
	for (auto c_ : conditions) {
		if (!c_.is_met(obs))
			return false;
	}
	return true;
}

BotAgent* Trigger::getAgent() {
	return agent;
}

StrategyOrder::StrategyOrder(BotAgent* agent_) {
	agent = agent_;
}

StrategyOrder::~StrategyOrder() {
	directives.clear();
}

bool StrategyOrder::execute() {
	return directives.front().execute(agent);
}

void StrategyOrder::enqueueDirective(Directive directive_) {
	// once a directive has been added to a StrategyOrder, it cannot be modified
	// this ensures we can look up whether the same directive already exists for a unit
	directive_.lock();
	directives.push_back(directive_);
}

void StrategyOrder::addTrigger(Trigger trigger_) {
	triggers.push_back(trigger_);
}

bool StrategyOrder::checkTriggerConditions() {
	for (Trigger t_ : triggers) {
		if (t_.check_conditions())
			return true;
	}
	return false;
}