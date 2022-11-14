#pragma once
#include "sc2api/sc2_api.h"
#include "Directive.h"

class Directive;
class BotAgent;

enum class SQUAD {
	SQUAD_STRUCTURE,
	SQUAD_WORKER,
	SQUAD_ARMY,
	SQUAD_PROXY,
	SQUAD_TOWNHALL
};

enum class FLAGS {
	IS_STRUCTURE,
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
	PERFORMING_ORDER
};

class SquadMember {
public:
	SquadMember(const sc2::Unit& unit_, SQUAD squad_type);
	void initVars();

	bool is_idle();
	bool has_flag(FLAGS flag);
	void assignDirective(Directive directive_);
	bool hasDefaultDirective();
	bool executeDefaultDirective(BotAgent* agent, const sc2::ObservationInterface* obs);


	// several public flags to filter our units
	bool has_default_directive;
	int group_id;
	sc2::Point2D birth_location;
	sc2::Point2D home_location;
	sc2::Point2D assigned_location;
	Directive* default_directive;
	const sc2::Unit& unit;
	std::unordered_set<FLAGS> flags;

private:
	
};