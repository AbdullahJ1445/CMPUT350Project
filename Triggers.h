#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "Agents.h"

class BotAgent;
class Directive;

enum class COND {
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
	StrategyOrder(BotAgent* agent_);
	bool execute(const sc2::ObservationInterface* obs);
	bool checkTriggerConditions(const sc2::ObservationInterface* obs);
	void addTriggerCondition(COND cond_type_, int cond_value_);
	void setDirective(Directive directive_);
	Trigger getTrigger();

private:
	BotAgent* agent;
	Trigger trigger;
	Directive* directive;
	bool has_directive;
};