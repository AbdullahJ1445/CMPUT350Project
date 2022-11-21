#pragma once

#include "Directive.h"
#include "Triggers.h"
#include "Mob.h"
#include "LocationHandler.h"
#include "Base.h"
#include "Strategy.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2api/sc2_client.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "MobHandler.h"

class Precept;
class Mob;
class Base;
class Strategy;
class MobHandler; 


class Human : public sc2::Agent {
public:
	virtual void OnGameStart() final {
		Debug()->DebugTextOut("Testing with Human");
		Debug()->SendDebug();
	}
};

class BasicSc2Bot : public sc2::Agent {
public:

	MobHandler* mobH;
	LocationHandler* locH;
	// public functions
	void setCurrentStrategy(Strategy* strategy_);
	void BasicSc2Bot::addStrat(Precept strategy);
	bool AssignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure);
	void storeDirective(Directive directive_);
	void storeStrategy(Strategy strategy_);
	Directive* getLastStoredDirective();
	void storeUnitType(std::string identifier_, sc2::UNIT_TYPEID unit_type_);
	void storeLocation(std::string identifier_, sc2::Point2D location_);
	sc2::UNIT_TYPEID getUnitType(std::string identifier_);
	sc2::Point2D getStoredLocation(std::string identifier_);
	int getMapIndex();

	// various bool functions
	bool have_upgrade(const sc2::UpgradeID upgrade_);
	bool can_unit_use_ability(const sc2::Unit& unit, const sc2::ABILITY_ID ability_);
	bool is_structure(const sc2::Unit* unit);
	bool is_mineral_patch(const sc2::Unit* unit_);
	bool is_geyser(const sc2::Unit* unit_);

	// various getters
	std::vector<sc2::Attribute> get_attributes(const sc2::Unit* unit);
	sc2::UnitTypeData getUnitTypeData(const sc2::Unit* unit);

	// public variables 
	//sc2::Point2D start_location;
	//sc2::Point2D proxy_location;
	//sc2::Point2D enemy_location;
	Mob* proxy_worker;
	std::unordered_map<size_t, Directive*> directive_by_id;
	Strategy* current_strategy;

private:

	// private functions
	void OnStep_100();
	void OnStep_1000();
	void initVariables();
	void initStartingUnits();

	// virtual functions 
	virtual void OnGameStart();
	virtual void OnStep();
	virtual void OnBuildingConstructionComplete(const sc2::Unit* unit);
	virtual void OnUnitCreated(const sc2::Unit* unit);
	virtual void OnUnitIdle(const sc2::Unit* unit);
	virtual void OnUnitDamaged(const sc2::Unit* unit, float health, float shields);
	virtual void OnUnitDestroyed(const sc2::Unit* unit);


	// data containers
	std::vector<Precept> strategies;
	std::vector<std::unique_ptr<Directive>> directive_storage;
	std::unordered_set<Directive*> stored_directives;
	std::unordered_map<std::string, sc2::UNIT_TYPEID> special_units;
	std::unordered_map<std::string, sc2::Point2D> special_locations;
	std::vector<std::unique_ptr<Strategy>> strategy_storage;

	// private variables
	int player_start_id;
	int enemy_start_id;
	std::string map_name;
	int map_index; // 1 = CactusValleyLE,  2 = BelShirVestigeLE,  3 = ProximaStationLE
};