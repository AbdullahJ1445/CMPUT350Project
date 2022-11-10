#pragma once

#include "Triggers.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2api/sc2_client.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"


class Human : public sc2::Agent {
public:
	virtual void OnGameStart() final {
		Debug()->DebugTextOut("Testing with Human");
		Debug()->SendDebug();

	}
};

class BotAgent : public sc2::Agent {
public:

	virtual void OnGameStart() final;
	virtual void OnStep() final;
	virtual void OnUnitIdle(const sc2::Unit* unit) final;

	/*
	virtual void OnUnitIdle(const sc2::Unit* unit) final {
		OnBotUnitIdle();
	}*/

private:
	sc2::Point2D start_location;
	sc2::Point2D army_rally;
	sc2::Point2D choke_point_1;
	sc2::Point2D choke_point_2;
	sc2::Point2D proxy_location;
	sc2::Point2D enemy_location;
	std::vector<StrategyOrder> strategies; 
	sc2::Unit proxy_worker;

	int player_start_id;
	int enemy_start_id;
	std::string map_name;
	int map_index; // 1 = CactusValleyLE,  2 = BelShirVestigeLE,  3 = ProximaStationLE


	void initVariables();
	void initLocations(int map_index, int p_id);
	int getPlayerIDForMap(int map_index, sc2::Point2D location);
	sc2::Point2D getNearestStartLocation(sc2::Point2D spot);
};