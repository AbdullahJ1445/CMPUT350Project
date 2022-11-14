#pragma once

#include "Directive.h"
#include "Triggers.h"
#include "Mob.h"
#include "Base.h"
#include "Strategy.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2api/sc2_client.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

class StrategyOrder;
class Mob;
class Base;
class Strategy;

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
	virtual void OnUnitDamaged(const sc2::Unit* unit, float health, float shields);
	bool have_upgrade(const sc2::UpgradeID upgrade_);
	bool AbilityAvailable(const sc2::Unit& unit, const sc2::ABILITY_ID ability_);
	bool AssignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure);
	Mob* getMob(const sc2::Unit& unit);
	std::vector<Mob*> getIdleWorkers();
	const sc2::Unit* FindNearestMineralPatch(const sc2::Point2D location);
	const sc2::Unit* FindNearestGeyser(const sc2::Point2D location);
	const sc2::Unit* FindNearestGasStructure(const sc2::Point2D location);
	void setCurrentStrategy(Strategy* strategy_);
	std::vector<Mob*> BotAgent::filter_by_flag(std::vector<Mob*> mobs_vector, FLAGS flag);
	std::vector<Mob*> BotAgent::filter_by_flags(std::vector<Mob*> mobs_vector, std::unordered_set<FLAGS> flag_list);
	std::vector<Base> bases;
	std::vector<Mob*> get_mobs();
	int BotAgent::get_index_of_closest_base(sc2::Point2D location_);
	void BotAgent::addStrat(StrategyOrder strategy);
	sc2::Point2D start_location;
	sc2::Point2D proxy_location;
	sc2::Point2D enemy_location;

private:
	
	sc2::Point2D army_rally;
	sc2::Point2D choke_point_1;
	sc2::Point2D choke_point_2;
	
	std::vector<StrategyOrder> strategies; 
	std::vector<Mob*> mobs;
	sc2::Unit proxy_worker;

	int player_start_id;
	int enemy_start_id;
	std::string map_name;
	int map_index; // 1 = CactusValleyLE,  2 = BelShirVestigeLE,  3 = ProximaStationLE
	Strategy* current_strategy;


	void initVariables();
	void initStartingUnits();
	void initLocations(int map_index, int p_id);
	int getPlayerIDForMap(int map_index, sc2::Point2D location);
	sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
};