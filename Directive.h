#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

class BotAgent;

class Directive {
	// An order which is executed upon a Trigger being met
public:

	enum DIR_TYPE {
		// different types of Directives
		UNIT_TYPE_SIMPLE_ACTION,
		SQUAD_MEMBER_SIMPLE_ACTION,
		WHOLE_SQUAD_SIMPLE_ACTION,
		UNIT_TYPE_TO_EXACT_LOCATION,
		SQUAD_MEMBER_TO_EXACT_LOCATION,
		WHOLE_SQUAD_TO_EXACT_LOCATION,
		UNIT_TYPE_TO_NEAR_LOCATION,
		SQUAD_MEMBER_TO_NEAR_LOCATION,
		WHOLE_SQUAD_TO_NEAR_LOCATION,
		UNIT_TYPE_TO_TARGET,
		SQUAD_MEMBER_TO_TARGET,
		WHOLE_SQUAD_TO_TARGET
	};

	Directive(DIR_TYPE dir_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_);

	Directive(DIR_TYPE dir_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_);

	Directive(DIR_TYPE dir_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Unit target_);

	// todo: implement constructors involving squads

	bool execute(BotAgent* agent, const sc2::ObservationInterface* obs);

private:
	DIR_TYPE dir_type;
	sc2::UNIT_TYPEID unit_type;
	sc2::ABILITY_ID ability;
	sc2::Point2D target_location;
	sc2::Unit target_unit;
};