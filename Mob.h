#pragma once
#include "sc2api/sc2_api.h"
#include "Directive.h"
#include "BasicSc2Bot.h"

class Directive;
class BasicSc2Bot;

enum class MOB {
	MOB_STRUCTURE,
	MOB_WORKER,
	MOB_ARMY,
	MOB_PROXY,
	MOB_TOWNHALL
};

enum class FLAGS {
	IS_STRUCTURE,
	IS_CONSTRUCTING,
	IS_TOWNHALL,
	IS_SUPPLY,
	IS_WORKER,
	IS_MINERAL_GATHERER,
	IS_GAS_GATHERER,
	IS_DEFENSE,
	IS_PROXY,
	IS_SIEGE,
	IS_ALERT,
	IS_ATTACKER,
	IS_ATTACKING,
	IS_FLYING,
	IS_INVISIBLE,
	BUILDING_GAS,
	IS_BUILDING_STRUCTURE,
	IS_IDLE,
	SHORT_RANGE,
	GROUND
};

class Mob {
public:
	Mob(const sc2::Unit& unit_, MOB mobs_type);
	void initVars();
	bool isIdle();
	bool hasFlag(FLAGS flag);
	void assignDefaultDirective(Directive directive_);
	void assignDirective(Directive* directive_);
	void unassignDirective();
	bool hasDefaultDirective();
	bool hasBundledDirective();
	bool hasCurrentDirective();
	bool executeDefaultDirective(BasicSc2Bot* agent);
	void disableDefaultDirective();
	Directive popBundledDirective();
	bool isCarryingMinerals();
	bool isCarryingGas();
	void setFlag(FLAGS flag);
	void removeFlag(FLAGS flag);
	sc2::Point2D getBirthLocation();
	sc2::Point2D getHomeLocation();
	sc2::Point2D getAssignedLocation();
	void setHomeLocation(sc2::Point2D location);
	void setAssignedLocation(sc2::Point2D location);
	void bundleDirectives(std::vector<Directive> dir_vec);
	std::unordered_set<FLAGS> getFlags();
	sc2::Tag getTag();
	bool setCurrentDirective(Directive* directive_);
	Directive* getCurrentDirective();
	bool operator<(const Mob& mob) const { return tag < mob.tag; }
	const sc2::Unit& unit;
	
private:
	std::unordered_set<FLAGS> flags;
	sc2::Point2D birth_location;
	sc2::Point2D home_location;
	sc2::Point2D assigned_location;
	sc2::Tag tag; // a unique identifier given to units
	bool has_default_directive;
	bool has_bundled_directive;
	bool has_current_directive;
	Directive* default_directive;
	Directive* bundled_directive;
	Directive* current_directive;
};