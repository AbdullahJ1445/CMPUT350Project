#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "Squad.h"

# define M_PI           3.14159265358979323846
# define DEFAULT_RADIUS 12.0f // should be equal to the square root of DEFAULT_SQ_DISTANCE in Triggers.h for best functionality

class BotAgent;
enum class FLAGS;

class Directive {
	// An order which is executed upon a Trigger being met
public:
	enum ASSIGNEE {
		// who the action should be assigned to
		DEFAULT_DIRECTIVE,
		UNIT_TYPE,
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

	Directive(const Directive& rhs);

	// todo: implement constructors involving squads

	bool execute(BotAgent* agent, const sc2::ObservationInterface* obs);
	bool executeForUnit(BotAgent* agent, const sc2::ObservationInterface* obs, const sc2::Unit& unit);
	static sc2::Point2D uniform_random_point_in_circle(sc2::Point2D center, float radius);
	void setDefault();

private:
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
};