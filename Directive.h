#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "Mob.h"

# define M_PI           3.14159265358979323846
# define DEFAULT_RADIUS 12.0f 

class BotAgent;
class Mob;
enum class FLAGS;

class Directive {
	// An order which is executed upon a Trigger being met
public:
	enum ASSIGNEE {
		// who the action should be assigned to
		DEFAULT_DIRECTIVE,
		UNIT_TYPE,
		UNIT_TYPE_NEAR_LOCATION,
		MATCH_FLAGS,
		MATCH_FLAGS_NEAR_LOCATION
	};
	enum ACTION_TYPE {
		// types of action to be performed
		SIMPLE_ACTION,
		EXACT_LOCATION,
		NEAR_LOCATION,
		TARGET_UNIT,
		TARGET_UNIT_NEAR_LOCATION,
		GET_MINERALS_NEAR_LOCATION,
		GET_GAS_NEAR_LOCATION
	};

	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_, 
		sc2::Point2D assignee_location_, sc2::Point2D target_location_, float assignee_proximity_=DEFAULT_RADIUS, float target_proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Unit target_);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::Point2D location_, float proximity_ =DEFAULT_RADIUS);

	bool execute(BotAgent* agent, const sc2::ObservationInterface* obs);
	bool executeForUnit(BotAgent* agent, const sc2::ObservationInterface* obs, const sc2::Unit& unit);
	static sc2::Point2D uniform_random_point_in_circle(sc2::Point2D center, float radius);
	void setDefault();
	void enqueueDirective(Directive directive_);
	bool hasQueuedDirective();

private:

	bool execute_simple_action_for_unit_type(BotAgent* agent);
	bool execute_build_gas_structure(BotAgent* agent);
	bool execute_protoss_nexus_chronoboost(BotAgent* agent);
	bool execute_match_flags(BotAgent* agent);
	bool execute_order_for_unit_type_with_location(BotAgent* agent);
	bool is_any_executing_order(std::vector<Mob*> mobs_vector, sc2::ABILITY_ID ability_);
	Mob* get_closest_to_location(std::vector<Mob*> mobs_vector, sc2::Point2D pos_);
	std::vector<Mob*> filter_near_location(std::vector<Mob*> mobs_vector, sc2::Point2D pos_, float radius_);
	std::vector<Mob*> filter_by_unit_type(std::vector<Mob*> mobs_vector, sc2::UNIT_TYPEID unit_type_);
	std::vector<Mob*> filter_idle(std::vector<Mob*> mobs_vector);

	ASSIGNEE assignee;
	ACTION_TYPE action_type;
	sc2::UNIT_TYPEID unit_type;
	sc2::ABILITY_ID ability;
	sc2::Point2D assignee_location;
	sc2::Point2D target_location;
	sc2::Unit target_unit;
	float assignee_proximity;
	float proximity;
	std::unordered_set<FLAGS> flags;
	std::vector<Directive> order_queue;
};