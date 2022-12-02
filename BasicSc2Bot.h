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
	void BasicSc2Bot::addStrat(Precept precept_);
	bool assignNearbyWorkerToGasStructure(const sc2::Unit& gas_structure);
	void storeDirective(Directive directive_);
	void storeStrategy(Strategy strategy_);
	Directive* getLastStoredDirective();
	void storeUnitType(std::string identifier_, sc2::UNIT_TYPEID unit_type_);
	void storeLocation(std::string identifier_, sc2::Point2D location_);
	sc2::UNIT_TYPEID getUnitType(std::string identifier_);
	sc2::Point2D getStoredLocation(std::string identifier_);
	int getMapIndex();

	// various bool functions
	bool haveUpgrade(const sc2::UpgradeID upgrade_);
	bool canUnitUseAbility(const sc2::Unit& unit, const sc2::ABILITY_ID ability_);
	bool isStructure(const sc2::Unit* unit);
	bool isMineralPatch(const sc2::Unit* unit_);
	bool isGeyser(const sc2::Unit* unit_);

	// various getters
	std::vector<sc2::Attribute> getAttributes(const sc2::Unit* unit);
	sc2::UnitTypeData getUnitTypeData(const sc2::Unit* unit);
	int getMineralCost(const sc2::Unit* unit);
	int getGasCost(const sc2::Unit* unit);
	int getFoodCost(const sc2::Unit* unit);
	float getValue(const sc2::Unit* unit);

	// public variables 
	//sc2::Point2D start_location;
	//sc2::Point2D proxy_location;
	//sc2::Point2D enemy_location;
	Mob* proxy_worker;
	std::unordered_map<size_t, Directive*> directive_by_id;
	Strategy* current_strategy;

private:

	// private functions
	void onStep_100(const sc2::ObservationInterface* obs);
	void onStep_1000(const sc2::ObservationInterface* obs);
	void initVariables();
	void initStartingUnits();
	bool addEnemyUnit(const sc2::Unit* unit);

	// virtual functions 
	virtual void onGameStart();
	virtual void onStep();
	virtual void onBuildingConstructionComplete(const sc2::Unit* unit);
	virtual void onUnitCreated(const sc2::Unit* unit);
	virtual void onUnitIdle(const sc2::Unit* unit);
	virtual void onUnitDamaged(const sc2::Unit* unit, float health, float shields);
	virtual void onUnitDestroyed(const sc2::Unit* unit);
	virtual void onUnitEnterVision(const sc2::Unit* unit);


	// data containers
	std::vector<Precept> precepts_onstep;
	std::vector<std::unique_ptr<Directive>> directive_storage;
	std::unordered_set<Directive*> stored_directives;
	std::unordered_map<std::string, sc2::UNIT_TYPEID> special_units;
	std::unordered_map<std::string, sc2::Point2D> special_locations;
	std::vector<std::unique_ptr<Strategy>> strategy_storage;
	std::vector<sc2::UNIT_TYPEID> data_buildings;
	std::unordered_set<const sc2::Unit*> enemy_units;
	std::unordered_map<sc2::Tag, const sc2::Unit*> enemy_unit_by_tag;
	std::unordered_map<int, int> mineral_cost;
	std::unordered_map<int, int> gas_cost;
	std::unordered_map<int, int> food_cost;

	// private variables
	int player_start_id;
	int enemy_start_id;
	std::string map_name;
	int map_index; // 1 = CactusValleyLE,  2 = BelShirVestigeLE,  3 = ProximaStationLE
};