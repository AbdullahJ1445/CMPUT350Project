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
	MIN_FOOD_USED,
	MIN_FOOD_CAP,
	MAX_MINERALS,
	MAX_GAS,
	MAX_TIME,
	MAX_FOOD,
	MAX_FOOD_USED,
	MAX_FOOD_CAP,
	MIN_MINERALS_MINED,
	MAX_MINERALS_MINED,
	MAX_UNIT_OF_TYPE,
	MIN_UNIT_OF_TYPE,
	MAX_UNIT_OF_TYPE_NEAR_LOCATION,
	MIN_UNIT_OF_TYPE_NEAR_LOCATION,
	MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION,
	MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION,
	HAS_ABILITY_READY,
	BASE_IS_ACTIVE,
	HAVE_UPGRADE
};

class Trigger {
public:
	class TriggerCondition;

	Trigger(BotAgent* agent_);
	void add_condition(TriggerCondition tc_);
	void add_condition(COND cond_type_, int cond_value_);
	void add_condition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_);
	void add_condition(COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_=true);
	void add_condition(COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_=true);
	void add_condition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
	bool check_conditions(const sc2::ObservationInterface* obs);
	BotAgent* getAgent();

private:
	std::vector<TriggerCondition> conditions;
	BotAgent* agent;

	class TriggerCondition {
	public:
		TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_);
		TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_);
		TriggerCondition(BotAgent* agent_, COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_=true);
		TriggerCondition(BotAgent* agent_, COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_=true);
		TriggerCondition(BotAgent* agent_, COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
		bool is_met(const sc2::ObservationInterface* obs);


	private:
		COND cond_type;
		int cond_value;
		sc2::UNIT_TYPEID unit_of_type;
		sc2::Point2D location_for_counting_units;
		float distance_squared;
		BotAgent* agent;
		bool is_true;
		sc2::UPGRADE_ID upgrade_id;
		sc2::ABILITY_ID ability_id;
	};

};

class StrategyOrder {
public:
	StrategyOrder(BotAgent* agent_);
	~StrategyOrder();
	bool execute(const sc2::ObservationInterface* obs);
	bool checkTriggerConditions(const sc2::ObservationInterface* obs);
	void addTrigger(Trigger trigger_);
	void setDirective(Directive directive_);

private:
	BotAgent* agent;
	std::vector<Trigger> triggers;
	std::vector<Directive*> directives;
	bool has_directive;
};