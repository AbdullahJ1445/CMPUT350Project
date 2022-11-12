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
	MIN_MINERALS,
	MIN_GAS,
	MIN_TIME,
	MIN_FOOD,
	MIN_FOOD_CAP,
	MAX_MINERALS,
	MAX_GAS,
	MAX_TIME,
	MAX_FOOD,
	MAX_FOOD_CAP,
	MAX_UNIT_OF_TYPE,
	MIN_UNIT_OF_TYPE,
	MAX_UNIT_OF_TYPE_NEAR_LOCATION,
	MIN_UNIT_OF_TYPE_NEAR_LOCATION
};

class TriggerCondition {
public:
	TriggerCondition(COND cond_type_, int cond_value_);
	TriggerCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_);
	TriggerCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
	bool is_met(const sc2::ObservationInterface* obs);


private:
	COND cond_type;
	int cond_value;
	sc2::UNIT_TYPEID unit_of_type;
	sc2::Point2D location_for_counting_units;
	float distance_squared;
};

class Trigger {
public:
	Trigger();
	void add_condition(TriggerCondition tc_);
	void add_condition(COND cond_type_, int cond_value_);
	void add_condition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_);
	void add_condition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
	bool check_conditions(const sc2::ObservationInterface* obs);

private:
	std::vector<TriggerCondition> conditions;
};

class StrategyOrder {
public:
	StrategyOrder(BotAgent* agent_);
	~StrategyOrder();
	bool execute(const sc2::ObservationInterface* obs);
	bool checkTriggerConditions(const sc2::ObservationInterface* obs);
	void setTrigger(Trigger trigger_);
	void addTriggerCondition(COND cond_type_, int cond_value_);
	void addTriggerCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_);
	void addTriggerCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
	void setDirective(Directive directive_);
	void addDirective(Directive directive_);
	Trigger getTrigger();

private:
	BotAgent* agent;
	Trigger trigger;
	std::vector<Directive*> directives;
	bool has_directive;
};