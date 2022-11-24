#pragma once

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "Mob.h"
#include <functional>
#include "Strategy.h"  // temp
//#include "LocationHandler.h"

# define M_PI					3.14159265358979323846 // pi
# define DEFAULT_RADIUS			12.0f // maximum distance for operation
# define USE_DEFINED_ABILITY	sc2::ABILITY_ID::EXPERIMENTALPLASMAGUN // value passed to issueOrder when no ability override specified
# define INVALID_POINT			sc2::Point2D(-1.11, -1.11) // value understood to mean a point is invalid, or uninitialized
# define SEND_HOME				sc2::Point2D(-9.64727, -9.64727) // value understood to mean the unit's home location
# define INVALID_RADIUS			-1.1F // value understood to mean a radius is invalid, or uninitialized
# define NO_POINT_FOUND			sc2::Point2D(-2.5252, -2.5252) // value understood to mean no valid point was found
# define ASSIGNED_LOCATION		sc2::Point2D(-5.0505, -5.0505) // value understood to mean a unit's assigned_location

# define TWI					sc2::ABILITY_ID::BUILD_TWILIGHTCOUNCIL

class BasicSc2Bot;
class Mob;
enum class FLAGS;
class Strategy; // temp



class Directive {
	// An order which is executed upon a Trigger being met
public:
	enum ASSIGNEE {
		// who the action should be assigned to
		DEFAULT_DIRECTIVE,
		UNIT_TYPE,
		UNIT_TYPE_NEAR_LOCATION,
		MATCH_FLAGS,
		MATCH_FLAGS_NEAR_LOCATION,
		UNITS_IN_GROUP,
		UNIT_TYPE_IN_GROUP
	};
	enum ACTION_TYPE {
		// types of action to be performed
		SIMPLE_ACTION,
		EXACT_LOCATION,
		NEAR_LOCATION,
		TARGET_UNIT,
		TARGET_UNIT_NEAR_LOCATION,
		GET_MINERALS_NEAR_LOCATION,
		GET_GAS_NEAR_LOCATION,
		DISABLE_DEFAULT_DIRECTIVE,
		ADD_TO_GROUP,
		REMOVE_FROM_GROUP
	};

	Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, float assignee_proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float assignee_proximity_=DEFAULT_RADIUS, float target_proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, float assignee_proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, std::unordered_set<FLAGS> flags_, sc2::ABILITY_ID ability_, 
		sc2::Point2D assignee_location_, sc2::Point2D target_location_, float assignee_proximity_=DEFAULT_RADIUS, float target_proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D location_, float proximity_=DEFAULT_RADIUS);
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Unit* target_);
	Directive(ASSIGNEE assignee_, sc2::Point2D assignee_location_, ACTION_TYPE action_type_, std::string group_name_, sc2::UNIT_TYPEID unit_type_, float assignee_proximity_=DEFAULT_RADIUS);


	//Directive(const Directive& d);
	//Directive& operator=(const Directive& d);

	bool execute(BasicSc2Bot* agent);
	bool executeForMob(BasicSc2Bot* agent, Mob* mob_);
	static sc2::Point2D uniform_random_point_in_circle(sc2::Point2D center, float radius);
	bool setDefault();
	bool bundleDirective(Directive directive_);
	void lock();
	bool assignMob(Mob* mob_);
	void unassignMob(Mob* mob_);
	void setTargetLocationFunction(Strategy* strat_, BasicSc2Bot* agent_, std::function<sc2::Point2D()> function_);
	void setAssigneeLocationFunction(BasicSc2Bot* agent_, std::function<sc2::Point2D()> function_);
	bool allowMultiple(bool is_true=true);
	void setContinuous(bool is_true=true);
	void setDebug(bool is_true=true);
	bool allowsMultiple();
	bool hasAssignedMob();
	int getTargetUpdateIterationID();
	int getAssigneeUpdateIterationID();
	sc2::ABILITY_ID getAbilityID();
	std::unordered_set<Mob*> getAssignedMobs();
	static Mob* getClosestToLocation(std::unordered_set<Mob*> mobs_set, sc2::Point2D pos_);
	size_t getID();
	Strategy* strategy_ref;    // testing this pointer
	
private:

	// generic constructor delegated by others
	Directive(ASSIGNEE assignee_, ACTION_TYPE action_type_, sc2::UNIT_TYPEID unit_type_, sc2::ABILITY_ID ability_, sc2::Point2D assignee_location_,
		sc2::Point2D target_location_, float assignee_proximity_, float target_proximity_, std::unordered_set<FLAGS> flags_, sc2::Unit* unit_, std::string group_name_);

	bool executeSimpleActionForUnitType(BasicSc2Bot* agent);
	bool executeBuildGasStructure(BasicSc2Bot* agent);
	bool execute_protoss_nexus_chronoboost(BasicSc2Bot* agent);
	bool executeMatchFlags(BasicSc2Bot* agent);
	bool executeOrderForUnitTypeWithLocation(BasicSc2Bot* agent);
	bool haveBundle();
	bool ifAnyOnRouteToBuild(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_);
	bool isBuildingStructure(BasicSc2Bot* agent, Mob* mob_);
	bool hasBuildOrder(Mob* mob_);
	std::unordered_set<Mob*> filterByHasAbility(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_set, sc2::ABILITY_ID ability_);
	bool isExecutingOrder(std::unordered_set<Mob*> mobs_set, sc2::ABILITY_ID ability_);
	std::unordered_set<Mob*> filterNearLocation(std::unordered_set<Mob*> mobs_set, sc2::Point2D pos_, float radius_);
	std::unordered_set<Mob*> filterByUnitType(std::unordered_set<Mob*> mobs_set, sc2::UNIT_TYPEID unit_type_);
	std::unordered_set<Mob*> filterNotBuildingStructure(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_set);
	std::unordered_set<Mob*> filterIdle(std::unordered_set<Mob*> mobs_set);
	std::unordered_set<Mob*> filterNotAssignedToThis(std::unordered_set<Mob*> mobs_set);
	Mob* getRandomMobFromSet(std::unordered_set<Mob*> mob_set);

	void updateTargetLocation(BasicSc2Bot* agent_);
	void updateAssigneeLocation(BasicSc2Bot* agent_);

	bool _genericIssueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, sc2::Point2D target_loc_, const sc2::Unit* target_unit_, bool queued_=false, sc2::ABILITY_ID ability_=USE_DEFINED_ABILITY);
	bool issueOrder(BasicSc2Bot* agent, Mob* mob_, bool queued_=false, sc2::ABILITY_ID ability_=USE_DEFINED_ABILITY);
	bool issueOrder(BasicSc2Bot* agent, Mob* mob_, sc2::Point2D target_loc_, bool queued_=false, sc2::ABILITY_ID ability_= USE_DEFINED_ABILITY);
	bool issueOrder(BasicSc2Bot* agent, Mob* mob_, const sc2::Unit* target_unit_, bool queued_=false, sc2::ABILITY_ID ability_ = USE_DEFINED_ABILITY);
	bool issueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, bool queued_=false, sc2::ABILITY_ID ability_ = USE_DEFINED_ABILITY);
	bool issueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, sc2::Point2D target_loc_, bool queued_=false, sc2::ABILITY_ID ability_ = USE_DEFINED_ABILITY);
	bool issueOrder(BasicSc2Bot* agent, std::unordered_set<Mob*> mobs_, const sc2::Unit* target_unit_, bool queued_=false, sc2::ABILITY_ID ability_ = USE_DEFINED_ABILITY);

	bool locked;
	bool allow_multiple;
	bool update_target_location;
	bool update_assignee_location;
	bool continuous_update;
	int target_update_iter_id;		// increments by one whenever target location updates to a new value
	int assignee_update_iter_id;	// increments by one whenever target location updates to a new value

	std::function<sc2::Point2D(void)> target_location_function;
	std::function<sc2::Point2D(void)> assignee_location_function;
	std::function<Strategy* ()> test_function;

	bool debug;

	ASSIGNEE assignee;
	ACTION_TYPE action_type;
	sc2::UNIT_TYPEID unit_type;
	sc2::ABILITY_ID ability;
	size_t id; // unique identifier
	
	sc2::Point2D assignee_location;
	sc2::Point2D target_location;
	sc2::Unit* target_unit;
	float assignee_proximity;
	float proximity;
	std::unordered_set<FLAGS> flags;
	std::vector<Directive> directive_bundle;
	std::unordered_set<Mob*> assigned_mobs;
	std::string group_name;
};