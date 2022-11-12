#pragma once

#include "Directive.h"
#include "Triggers.h"
#include "Squad.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2api/sc2_client.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

class StrategyOrder;
class SquadMember;

class Human : public sc2::Agent {
public:
	virtual void OnGameStart() final {
		Debug()->DebugTextOut("Testing with Human");
		Debug()->SendDebug();
	}
};

class BotAgent : public sc2::Agent {
public:

	void OnStep_1000();
	void OnStep_100();
	virtual void OnGameStart() final;
	virtual void OnStep() final;
	//virtual void OnUnitIdle(const sc2::Unit& unit) final;
	virtual void OnBuildingConstructionComplete(const sc2::Unit* unit) final;
	virtual void OnUnitCreated(const sc2::Unit* unit);
	bool AssignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure);
	SquadMember* getSquadMember(const sc2::Unit& unit);
	std::vector<SquadMember*> getIdleWorkers();
	const sc2::Unit* FindNearestMineralPatch(const sc2::Point2D location);
	const sc2::Unit* FindNearestGeyser(const sc2::Point2D location);
	const sc2::Unit* FindNearestGasStructure(const sc2::Point2D location);
	std::vector<SquadMember*> BotAgent::filter_by_flag(std::vector<SquadMember*> squad_vector, FLAGS flag);
	std::vector<SquadMember*> BotAgent::filter_by_flags(std::vector<SquadMember*> squad_vector, std::vector<FLAGS> flag_list);

private:
	sc2::Point2D start_location;
	sc2::Point2D army_rally;
	sc2::Point2D choke_point_1;
	sc2::Point2D choke_point_2;
	sc2::Point2D proxy_location;
	sc2::Point2D enemy_location;
	std::vector<StrategyOrder> strategies; 
	std::vector<SquadMember*> squad_members;
	sc2::Unit proxy_worker;

	int player_start_id;
	int enemy_start_id;
	std::string map_name;
	int map_index; // 1 = CactusValleyLE,  2 = BelShirVestigeLE,  3 = ProximaStationLE


	void initVariables();
	void initStartingUnits();
	void initLocations(int map_index, int p_id);
	int getPlayerIDForMap(int map_index, sc2::Point2D location);
	sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
};