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

	// public functions
	void setCurrentStrategy(Strategy* strategy_);
	void BotAgent::addStrat(StrategyOrder strategy);
	void set_mob_idle(Mob* mob_, bool is_true = true);
	bool AssignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure);
	bool addMob(Mob mob_);

	// various bool functions
	bool have_upgrade(const sc2::UpgradeID upgrade_);
	bool can_unit_use_ability(const sc2::Unit& unit, const sc2::ABILITY_ID ability_);
	bool mob_exists(const sc2::Unit& unit);
	bool is_structure(const sc2::Unit* unit);
	bool is_mineral_patch(const sc2::Unit* unit_);
	bool is_geyser(const sc2::Unit* unit_);

	// various getters etc.
	Mob& getMob(const sc2::Unit& unit);
	std::unordered_set<Mob*> getIdleWorkers();
	std::vector<sc2::Attribute> get_attributes(const sc2::Unit* unit);
	sc2::UnitTypeData getUnitTypeData(const sc2::Unit* unit);
	std::unordered_set<Mob*> BotAgent::filter_by_flag(std::unordered_set<Mob*> mobs_set, FLAGS flag, bool is_true=true);
	std::unordered_set<Mob*> BotAgent::filter_by_flags(std::unordered_set<Mob*> mobs_set, std::unordered_set<FLAGS> flag_list, bool is_true=true);
	std::unordered_set<Mob*> get_mobs();
	int BotAgent::get_index_of_closest_base(sc2::Point2D location_);
	sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
	const sc2::Unit* FindNearestMineralPatch(const sc2::Point2D location);
	const sc2::Unit* FindNearestGeyser(const sc2::Point2D location);
	const sc2::Unit* FindNearestGasStructure(const sc2::Point2D location);
	const sc2::Unit* FindNearestTownhall(const sc2::Point2D location);

	// public variables 
	std::vector<Base> bases;
	sc2::Point2D start_location;
	sc2::Point2D proxy_location;
	sc2::Point2D enemy_location;
	Mob* proxy_worker;

private:

	// private functions
	void OnStep_100();
	void OnStep_1000();
	void initVariables();
	void initStartingUnits();
	void initLocations(int map_index, int p_id);
	int getPlayerIDForMap(int map_index, sc2::Point2D location);

	// virtual functions 
	virtual void OnGameStart() final;
	virtual void OnStep() final;
	virtual void OnBuildingConstructionComplete(const sc2::Unit* unit) final;
	virtual void OnUnitCreated(const sc2::Unit* unit);
	virtual void OnUnitIdle(const sc2::Unit* unit) final;
	virtual void OnUnitDamaged(const sc2::Unit* unit, float health, float shields);

	// data containers
	std::vector<StrategyOrder> strategies;
	std::vector<std::shared_ptr<Mob>> mobs_storage;
	std::unordered_set<Mob*> mobs;
	std::unordered_set<Mob*> idle_mobs;
	std::unordered_map<sc2::Tag, Mob*> mob_by_tag;

	// private variables
	int player_start_id;
	int enemy_start_id;
	std::string map_name;
	int map_index; // 1 = CactusValleyLE,  2 = BelShirVestigeLE,  3 = ProximaStationLE
	Strategy* current_strategy;
};