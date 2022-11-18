#include "Strategy.h"

Strategy::Strategy(BotAgent* bot_) {
	bot = bot_;
}

void Strategy::loadGameSettings(int* map_index, sc2::Race* bot_race, sc2::Race* opp_race, bool* human_player, bool* fullscreen, bool* realtime) {
	// Adjust These Settings to fit your desired strategy

	// map_index:
	//	1: CactusValleyLE
	//  2: BelShirVestigeLE
	//  3: ProximaStationLE
	*map_index = 1;
	*bot_race = sc2::Race::Terran;
	*opp_race = sc2::Race::Protoss;
	*human_player = false;
	*fullscreen = false;
	*realtime = true;
}


void Strategy::loadStrategies() {

	// (1) Create a StrategyOrder - Contains the Full Order
	// (2) Create a Directive - What should happen when a trigger is met
	// (3) Create a Trigger - A trigger contains TriggerConditions, and is satisfied when all conditions are met
	// (4) Add TriggerConditions to the Trigger by using .add_condtion()
	// (5) repeat 3 and 4 if you wish for this directive to be executed under different conditions
	// (6) set the directive with .addDirective()
	// (7) add the trigger(s) with .addTrigger()
	// (8) add the StrategyOrder to the bot with bot->addStrat()
	
	
	/*  This shows how to bundle several directives together. */
	/*
	{
		StrategyOrder test(bot);
		Directive big_d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->bases[0].get_build_area(0));
		Directive d2(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->bases[0].get_build_area(0));
		Directive d3(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->bases[0].get_build_area(0));
		Directive d4(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->bases[0].get_build_area(0));
		big_d.addDirective(d2);
		big_d.addDirective(d3);
		big_d.addDirective(d4);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 400);
		test.addDirective(big_d);
		test.addTrigger(t);
		bot->addStrat(test);
	} 
	*/
	/*
	{
		StrategyOrder base_scv(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER, sc2::ABILITY_ID::TRAIN_SCV);
		Trigger t(bot);
		t.add_condition(COND::MIN_FOOD, 1);
		t.add_condition(COND::MIN_MINERALS, 50);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 15, sc2::UNIT_TYPEID::TERRAN_SCV, bot->bases[0].get_townhall(), 18.0f);
		base_scv.addDirective(d);
		base_scv.addTrigger(t);
		Trigger t2(bot);
		t2.add_condition(COND::MIN_FOOD, 1);
		t2.add_condition(COND::MIN_MINERALS, 50);
		t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 15, sc2::UNIT_TYPEID::TERRAN_SCV, bot->bases[0].get_townhall(), 18.0f);
		t2.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::TERRAN_REFINERY, bot->bases[0].get_townhall());
		base_scv.addTrigger(t2);
		bot->addStrat(base_scv);
	}  */
	{
		StrategyOrder build_supply(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::TERRAN_SCV, sc2::ABILITY_ID::BUILD_SUPPLYDEPOT, bot->bases[0].get_defend_point(0), 7.0f);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 100);
		build_supply.addDirective(d);
		build_supply.addTrigger(t);
		bot->addStrat(build_supply);
	}
	/*
	{
		StrategyOrder build_supply_and_barracks(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::TERRAN_SCV, sc2::ABILITY_ID::BUILD_SUPPLYDEPOT, bot->bases[0].get_defend_point(0), 7.0f);
		Directive d2(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::TERRAN_SCV, sc2::ABILITY_ID::BUILD_BARRACKS, bot->bases[0].get_defend_point(0), 7.0f);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 250);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT);
		build_supply_and_barracks.addDirective(d);
		build_supply_and_barracks.addDirective(d2);
		build_supply_and_barracks.addTrigger(t);
		bot->addStrat(build_supply_and_barracks);
	}
	*/
	
	
	/*
	{
		StrategyOrder choke_supply_depot(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::TERRAN_SCV, sc2::ABILITY_ID::BUILD_SUPPLYDEPOT, bot->bases[0].get_defend_point(0), 7.0f);
		Trigger t(bot);
		//t.add_condition(COND::MIN_MINERALS, 100);
		t.add_condition(COND::MAX_FOOD, 4);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, bot->bases[0].get_defend_point(0), 8.0f);
		choke_supply_depot.addDirective(d);
		choke_supply_depot.addTrigger(t);
		bot->addStrat(choke_supply_depot);
	}
	
	{
		StrategyOrder base_supply_depot(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::TERRAN_SCV, sc2::ABILITY_ID::BUILD_SUPPLYDEPOT, bot->bases[0].get_defend_point(0), 7.0f);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 300);
		//t.add_condition(COND::MAX_FOOD, 4);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 4, sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, bot->bases[0].get_build_area(0));
		base_supply_depot.addDirective(d);
		base_supply_depot.addTrigger(t);
		bot->addStrat(base_supply_depot);
	}
	{
		StrategyOrder base_supply_depot(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::TERRAN_SCV, sc2::ABILITY_ID::BUILD_BARRACKS, bot->bases[0].get_defend_point(0), 7.0f);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 301);
		//t.add_condition(COND::MAX_FOOD, 4);
		//t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 4, sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT, bot->bases[0].get_build_area(0));
		base_supply_depot.addDirective(d);
		base_supply_depot.addTrigger(t);
		bot->addStrat(base_supply_depot);
	}
	*/
}