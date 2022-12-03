#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "BasicSc2Bot.h"
#include "Mob.h"

# define DEFAULT_RADIUS 12.0f 

class BasicSc2Bot;
class Directive;
enum class FLAGS;

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
	MAX_UNIT_OF_TYPE_TOTAL,
	MIN_UNIT_OF_TYPE_TOTAL,
	MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION,
	MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION,
	MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION,
	MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION,
	MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION,
	MIN_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION,
	MIN_UNIT_WITH_FLAGS,
	MAX_UNIT_WITH_FLAGS,
	MIN_UNIT_WITH_FLAGS_NEAR_LOCATION,
	MAX_UNIT_WITH_FLAGS_NEAR_LOCATION,
	HAS_ABILITY_READY,
	BASE_IS_ACTIVE,
	HAVE_UPGRADE,
	MAX_UNITS_IN_GROUP,
	MIN_UNITS_IN_GROUP,
	MAX_UNITS_OF_TYPE_IN_GROUP,
	MIN_UNITS_OF_TYPE_IN_GROUP,
	MAX_UNITS_IN_GROUP_NEAR_LOCATION,
	MIN_UNITS_IN_GROUP_NEAR_LOCATION,
	MAX_UNITS_USING_ABILITY,
	MIN_UNITS_USING_ABILITY,
	MAX_DEAD_MOBS,
	MIN_DEAD_MOBS,
	ENEMY_RACE_ZERG,
	ENEMY_RACE_PROTOSS,
	ENEMY_RACE_TERRAN,
	ENEMY_RACE_UNKNOWN,
	THREAT_EXISTS_NEAR_LOCATION,
	MAX_NEUTRAL_UNIT_OF_TYPE,
	MIN_NEUTRAL_UNIT_OF_TYPE,
	MAX_ENEMY_UNITS_NEAR_LOCATION,
	MIN_ENEMY_UNITS_NEAR_LOCATION,
	TIMER_1_SET,
	TIMER_2_SET,
	TIMER_3_SET,
	TIMER_1_MIN_STEPS_PAST,
	TIMER_1_MAX_STEPS_PAST,
	TIMER_2_MIN_STEPS_PAST,
	TIMER_2_MAX_STEPS_PAST,
	TIMER_3_MIN_STEPS_PAST,
	TIMER_3_MAX_STEPS_PAST,
};

class Trigger {
public:
	class TriggerCondition;

	Trigger(BasicSc2Bot* agent_);
	void addCondition(TriggerCondition tc_);
	void addCondition(COND cond_type_, double cond_value_=0, bool is_true_=true);
	void addCondition(COND cond_type_, double cond_value_, sc2::UNIT_TYPEID unit_of_type_);
	void addCondition(COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_=true);
	void addCondition(COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_=true);
	void addCondition(COND cond_type_, sc2::Point2D location_, float radius=DEFAULT_RADIUS, bool is_true=true);
	void addCondition(COND cond_type_, double cond_value_, sc2::Point2D location_, float radius = DEFAULT_RADIUS, bool is_true = true);
	void addCondition(COND cond_type_, double cond_value_, std::unordered_set<FLAGS> flags_);
	void addCondition(COND cond_type_, double cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_, bool is_true_=true);
	void addCondition(COND cond_type_, double cond_value_, std::unordered_set<FLAGS> flags_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
	void addCondition(COND cond_type_, double cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
	bool checkConditions();
	void setDebug(bool is_true=true);
	BasicSc2Bot* getAgent();

	class TriggerCondition {
	public:
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, double cond_value_=0, bool is_true_=true);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, double cond_value_, sc2::UNIT_TYPEID unit_of_type_);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_=true);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_=true);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, sc2::Point2D location_, float radius=DEFAULT_RADIUS, bool is_true_=true);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, double cond_value_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS, bool is_true_=true);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, double cond_value_, std::unordered_set<FLAGS> flags_);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, double cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_, bool is_true_=true);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, double cond_value_, std::unordered_set<FLAGS> flags_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
		TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, double cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_=DEFAULT_RADIUS);
		bool is_met(const sc2::ObservationInterface* obs);
		void setDebug(bool isTrue=true);


	private:
		COND cond_type;
		double cond_value;
		sc2::UNIT_TYPEID unit_of_type;
		sc2::Point2D location;
		float radius;
		BasicSc2Bot* agent;
		bool debug;
		bool is_true;
		sc2::UPGRADE_ID upgrade_id;
		sc2::ABILITY_ID ability_id;
		std::unordered_set<FLAGS> filter_flags;
	};

private:
	std::vector<TriggerCondition> conditions;
	BasicSc2Bot* agent;
	bool debug;
};

class Precept {
// Precepts are made up of one or more Directives
// those of which get executed when all the TriggerConditions of the associated Trigger are satisfied
public:
	Precept(BasicSc2Bot* agent_);
	~Precept();
	bool execute();
	bool checkTriggerConditions();
	void setDebug(bool is_true=true);
	void addTrigger(Trigger trigger_);
	void addDirective(Directive directive_);
	bool hasDirective();
	bool hasTrigger();

protected:
	BasicSc2Bot* agent;
	std::vector<Trigger> triggers;
	std::vector<Directive*> directives;
	bool has_directive;
	bool has_trigger;
	bool debug;
};