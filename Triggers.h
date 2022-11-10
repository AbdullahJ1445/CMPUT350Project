#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "Agents.h"

enum COND {
	// The types of trigger conditions
	MIN_MINERALS = 0,
	MIN_GAS = 1,
	MIN_TIME = 2,
	MIN_FOOD = 3,
	MIN_FOOD_CAP = 4,
	MAX_MINERALS = 5,
	MAX_GAS = 6,
	MAX_TIME = 7,
	MAX_FOOD = 8,
	MAX_FOOD_CAP = 9
};

class TriggerCondition {
public:
	TriggerCondition(COND cond_type_, int cond_value_);
	bool is_met(const sc2::ObservationInterface* obs);

private:
	COND cond_type;
	int cond_value;
};

class Trigger {
public:
	Trigger();
	void add_condition(TriggerCondition tc_);
	bool check_conditions(const sc2::ObservationInterface* obs);

private:
	std::vector<TriggerCondition> conditions;
};

class StrategyOrder {
public:
	StrategyOrder(BotAgent* agent_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D unit_location_, sc2::Point2D target_location_);
	bool execute(const sc2::ObservationInterface* obs);
	bool checkTriggerConditions(const sc2::ObservationInterface* obs);
	void addTriggerCondition(COND cond_type_, int cond_value_);
	Trigger getTrigger();

private:
	BotAgent* agent;
	sc2::UNIT_TYPEID unit_type;
	sc2::ABILITY_ID ability;
	sc2::Point2D unit_location;
	sc2::Point2D target_location;
	Trigger trigger;
};