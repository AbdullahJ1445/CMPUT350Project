#include "Strategy.h"

Strategy::Strategy(BasicSc2Bot* bot_) {
	bot = bot_;
	strategy_ptr = this;
}

void Strategy::loadGameSettings(int* map_index, sc2::Race* bot_race, sc2::Race* opp_race, sc2::Difficulty* difficulty, bool* human_player, bool* fullscreen, bool* realtime) {
	// Adjust These Settings to fit your desired strategy

	// map_index:
	//	1: CactusValleyLE
	//  2: BelShirVestigeLE
	//  3: ProximaStationLE
	*map_index = 2;
	*bot_race = sc2::Race::Protoss;
	*opp_race = sc2::Race::Terran;
	*difficulty = sc2::Difficulty::VeryEasy;
	*human_player = false;
	*fullscreen = false;
	*realtime = false;

	// Note: do not use these, they do not work with the LadderInterface - specify commandline arguments
}


void Strategy::loadStrategies() {
	map_index = bot->getMapIndex();
	p_id = bot->locH->getPlayerIDForMap(map_index, bot->Observation()->GetStartLocation());

	// Cactus Valley Strategy
	if (map_index == 1) {
		// Cactus Valley cannot easily be cheesed unlike the 1v1 maps
		// We determined our best option to be turtling and building up a strong force to attack with

		// initialize cactus valley first structure locations

		if (p_id == 1) {
			bot->storeLocation("FORGE_1", sc2::Point2D(68.5, 148.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(68.0, 153.0));
			bot->storeLocation("PYLON_2", sc2::Point2D(68, 155));
			bot->storeLocation("CANNON_1", sc2::Point2D(66.0, 152));
			bot->storeLocation("CANNON_2", sc2::Point2D(71, 148));
			bot->storeLocation("SHIELD_2", sc2::Point2D(65, 149));
			bot->storeLocation("SHIELD_1", sc2::Point2D(70, 154.5));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(71.5, 150.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(67.5, 157.5));
			bot->storeLocation("FORCE_FIELD", sc2::Point2D(73.5, 152));
			bot->storeLocation("FF_CHECK", sc2::Point2D(71.5, 154));
		}
		if (p_id == 2) {
			bot->storeLocation("FORGE_1", sc2::Point2D(148.5, 123.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(153, 124));
			bot->storeLocation("PYLON_2", sc2::Point2D(155, 124));
			bot->storeLocation("CANNON_1", sc2::Point2D(152, 126));
			bot->storeLocation("CANNON_2", sc2::Point2D(148, 121));
			bot->storeLocation("SHIELD_2", sc2::Point2D(149, 127));
			bot->storeLocation("SHIELD_1", sc2::Point2D(154.5, 122));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(150.5, 120.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(157.5, 124.5));
			bot->storeLocation("FORCE_FIELD", sc2::Point2D(152, 118.5));
			bot->storeLocation("FF_CHECK", sc2::Point2D(154, 120.5));
		}
		if (p_id == 3) {
			bot->storeLocation("FORGE_1", sc2::Point2D(123.5, 43.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(124, 39));
			bot->storeLocation("PYLON_2", sc2::Point2D(124, 37));
			bot->storeLocation("CANNON_1", sc2::Point2D(126, 40));
			bot->storeLocation("CANNON_2", sc2::Point2D(121, 44));
			bot->storeLocation("SHIELD_2", sc2::Point2D(127, 43));
			bot->storeLocation("SHIELD_1", sc2::Point2D(122, 37.5));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(120.5, 41.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(124.5, 34.5));
			bot->storeLocation("FORCE_FIELD", sc2::Point2D(118.5, 40));
			bot->storeLocation("FF_CHECK", sc2::Point2D(120.5, 38));
		}
		if (p_id == 4) {
			bot->storeLocation("FORGE_1", sc2::Point2D(43.5, 68.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(39, 68));
			bot->storeLocation("PYLON_2", sc2::Point2D(37, 68));
			bot->storeLocation("CANNON_1", sc2::Point2D(40, 66));
			bot->storeLocation("CANNON_2", sc2::Point2D(44, 71));
			bot->storeLocation("SHIELD_2", sc2::Point2D(43, 65));
			bot->storeLocation("SHIELD_1", sc2::Point2D(37.5, 70));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(41.5, 71.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(34.5, 67.5));
			bot->storeLocation("FORCE_FIELD", sc2::Point2D(40, 73.5));
			bot->storeLocation("FF_CHECK", sc2::Point2D(38, 71.5));
		}
		bot->storeLocation("DEBUG_TEST", sc2::Point2D(-777, -777));

		{	// Train Probes at our main Nexus
			Precept base_probe(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 50);
			t.addCondition(COND::MIN_FOOD, 1);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 18, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[0].getTownhall(), 12.0f);
			base_probe.addDirective(d);
			base_probe.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 50);
			t2.addCondition(COND::MIN_FOOD, 1);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall(), 6.0F);
			base_probe.addTrigger(t2);
			bot->addStrat(base_probe);
		}

		bot->storeInt("_GRAB_WORKERS_ON_EXPAND", 15); // identifier use to determine number of workers to grab after expansion is built

		{	// Expand to the first expansion almost immediately
			Precept first_expansion(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_NEXUS, bot->locH->bases[1].getTownhall());
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 400);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall(), 4.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			first_expansion.addDirective(d);
			first_expansion.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 400);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall(), 4.0F);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t2.addCondition(COND::MIN_TIME, 3000);
			first_expansion.addTrigger(t2);
			bot->addStrat(first_expansion);
		}
		{	// Train Probes at Expansion
			Precept exp_probe(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[1].getTownhall(), Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 50);
			t.addCondition(COND::MIN_FOOD, 1);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 19, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[1].getTownhall(), 12.0f);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall(), 4.0f);
			exp_probe.addDirective(d);
			exp_probe.addTrigger(t);
			bot->addStrat(exp_probe);
		}
		{	// build pylon #1 at our pre-defined wall location
			Precept pylon_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->getStoredLocation("PYLON_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			//t.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 0.5F);
			pylon_1.addDirective(d);
			pylon_1.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 100);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall());
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 0.5F);
			pylon_1.addTrigger(t2);
			bot->addStrat(pylon_1);
		}
		{	// build forge #1 at our pre-defined wall location
			Precept forge_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_FORGE, bot->getStoredLocation("FORGE_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_TIME, 4000);
			forge_1.addDirective(d);
			forge_1.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall());
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MAX_TIME, 4000);
			forge_1.addTrigger(t2);
			bot->addStrat(forge_1);
		}
		{	// build forge #1 inside main base if the first forge falls
			Precept forge_backup(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_FORGE, bot->locH->bases[0].getBuildArea(0));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0));
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MIN_TIME, 4001);
			forge_backup.addDirective(d);
			forge_backup.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall());
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MIN_TIME, 4001);
			forge_backup.addTrigger(t2);
			bot->addStrat(forge_backup);
		}
		{	// build gateway #1 at our pre-defined wall location
			Precept gateway_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->getStoredLocation("GATEWAY_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			t.addCondition(COND::MAX_TIME, 4000);
			gateway_1.addDirective(d);
			gateway_1.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			t2.addCondition(COND::MAX_TIME, 4000);
			gateway_1.addTrigger(t2);
			bot->addStrat(gateway_1);
		}
		{	// build pylon #2 in our pre-defined wall location
			Precept pylon_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->getStoredLocation("PYLON_2"));
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_2"), 0.5F);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			pylon_2.addDirective(d);
			pylon_2.addTrigger(t);
			bot->addStrat(pylon_2);
		}
		{	// build cannon #1 at our pre-defined wall location
			Precept cannon_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->getStoredLocation("CANNON_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->getStoredLocation("CANNON_1"), 0.5F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->getStoredLocation("CANNON_1"), 0.5F);
			cannon_1.addDirective(d);
			cannon_1.addTrigger(t);
			bot->addStrat(cannon_1);
		}
		{	// build cannon #2 at our pre-defined wall location
			Precept cannon_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->getStoredLocation("CANNON_2"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ZEALOT);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_STALKER);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->getStoredLocation("CANNON_2"), 0.5F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->getStoredLocation("CANNON_2"), 0.5F);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t.addCondition(COND::MAX_TIME, 9000);
			cannon_2.addDirective(d);
			cannon_2.addTrigger(t);
			bot->addStrat(cannon_2);
		}
		{	// build our cybernetics core at our pre-defined wall location
			Precept cyber_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_CYBERNETICSCORE, bot->getStoredLocation("CYBER_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, bot->getStoredLocation("CYBER_1"), 0.5F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, bot->getStoredLocation("CYBER_1"), 0.5F);
			cyber_1.addDirective(d);
			cyber_1.addTrigger(t);
			bot->addStrat(cyber_1);
		}
		{	// Ensure there is at least one pylon at main base build area 0
			Precept main_pylon(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].getBuildArea(0), 4.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0), 6.0F);
			main_pylon.addDirective(d);
			main_pylon.addTrigger(t);
			bot->addStrat(main_pylon);
		}
		{	// Continuously build pylons in main base build area 0 when low on food (after first 4 are built)
			Precept main_pylon_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].getBuildArea(0), 12.0F);
			d.allowMultiple();
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MAX_FOOD, 6);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 8, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			main_pylon_2.addDirective(d);
			main_pylon_2.addTrigger(t);
			bot->addStrat(main_pylon_2);
		}
		{	// Ensure there is at least one pylon at main base build area 1
			Precept main_pylon_3(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].getBuildArea(1), 4.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(1), 6.0F);
			main_pylon_3.addDirective(d);
			main_pylon_3.addTrigger(t);
			bot->addStrat(main_pylon_3);
		}
		{	// Build 10th to 14th pylon at main base build area 2
			Precept main_pylon_4(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].getBuildArea(2), 12.0F);
			d.allowMultiple();
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MAX_FOOD, 7);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 9, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 14, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			main_pylon_4.addDirective(d);
			main_pylon_4.addTrigger(t);
			bot->addStrat(main_pylon_4);
		}
		{	// build gateways at main base build area 0
			Precept main_gateway(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->locH->bases[0].getBuildArea(0));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0), 8.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::ENEMY_RACE_ZERG);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			main_gateway.addDirective(d);
			main_gateway.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 100);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0), 8.0F);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t2.addCondition(COND::ENEMY_RACE_ZERG, 0, false);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			main_gateway.addTrigger(t2);
			bot->addStrat(main_gateway);
		}
		{	// build gateways at main base build area 1
			Precept gateway_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->locH->bases[0].getBuildArea(1), 10.0F);
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::MIN_MINERALS, 300);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t.addCondition(COND::ENEMY_RACE_ZERG);
			gateway_2.addDirective(d);
			gateway_2.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_FOOD, 4);
			t2.addCondition(COND::MIN_MINERALS, 300);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t2.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // build less gateways if not facing zerg
			gateway_2.addTrigger(t2);
			bot->addStrat(gateway_2);
		}
		{	// build robotics facilities in main base build area 1
			Precept robotics_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ROBOTICSFACILITY, bot->locH->bases[0].getBuildArea(1), 10.0F);
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 100);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t.addCondition(COND::ENEMY_RACE_ZERG); // if zerg require 4 gateways first
			robotics_1.addDirective(d);
			robotics_1.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_FOOD, 4);
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_GAS, 100);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t2.addCondition(COND::ENEMY_RACE_ZERG); // if zerg and have robo_bay and colossus
			robotics_1.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MIN_FOOD, 4);
			t3.addCondition(COND::MIN_MINERALS, 150);
			t3.addCondition(COND::MIN_GAS, 100);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t3.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // if not zerg start after 2 gateways
			robotics_1.addTrigger(t3);
			Trigger t4(bot);
			t4.addCondition(COND::MIN_FOOD, 4);
			t4.addCondition(COND::MIN_MINERALS, 150);
			t4.addCondition(COND::MIN_GAS, 100);
			t4.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 3, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t4.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t4.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // if not zerg and have robo_Bay and colossus
			robotics_1.addTrigger(t4);
			bot->addStrat(robotics_1);
		}
		{	// build robotics facilities in main base build area 2
			Precept robotics_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ROBOTICSFACILITY, bot->locH->bases[0].getBuildArea(2), 10.0F);
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 100);
			t.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // build more against non-zerg
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			robotics_2.addDirective(d);
			robotics_2.addTrigger(t);
			bot->addStrat(robotics_2);
		}
		{	// build a robotics bay in main base build area 2
			Precept robotics_bay(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ROBOTICSBAY, bot->locH->bases[0].getBuildArea(2), 10.0F);
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_FOOD_CAP, 60);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			robotics_bay.addDirective(d);
			robotics_bay.addTrigger(t);
			bot->addStrat(robotics_bay);
		}
		{	// train immortals
			Precept train_immortal(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, sc2::ABILITY_ID::TRAIN_IMMORTAL);
			d.allowMultiple();
			train_immortal.addDirective(d);
			Trigger t(bot);  // not zerg, no robotics bay
			t.addCondition(COND::MIN_MINERALS, 275);
			t.addCondition(COND::MIN_GAS, 100);
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::ENEMY_RACE_ZERG, 0, false);
			t.addCondition(COND::MAX_FOOD_USED, 115);
			t.addCondition(COND::MAX_TIME, 19999);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_immortal.addTrigger(t);
			Trigger t2(bot); // zerg, no robotics bay
			t2.addCondition(COND::MIN_MINERALS, 350);
			t2.addCondition(COND::MIN_GAS, 150);
			t2.addCondition(COND::MIN_FOOD, 4);
			t2.addCondition(COND::ENEMY_RACE_ZERG);
			t2.addCondition(COND::MAX_FOOD_USED, 115);
			t2.addCondition(COND::MAX_TIME, 19999);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_immortal.addTrigger(t2);
			Trigger t3(bot);  // not zerg, have robotics bay, no colossus
			t3.addCondition(COND::MIN_MINERALS, 275);
			t3.addCondition(COND::MIN_GAS, 100);
			t3.addCondition(COND::MIN_FOOD, 4);
			t3.addCondition(COND::ENEMY_RACE_ZERG, 0, false);
			t3.addCondition(COND::MAX_FOOD_USED, 115);
			t3.addCondition(COND::MAX_TIME, 19999);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, bot->locH->bases[1].getRallyPoint(), 30.0F);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_immortal.addTrigger(t3);
			Trigger t4(bot); // zerg, have robotics bay, no colossus
			t4.addCondition(COND::MIN_MINERALS, 350);
			t4.addCondition(COND::MIN_GAS, 150);
			t4.addCondition(COND::MIN_FOOD, 4);
			t4.addCondition(COND::ENEMY_RACE_ZERG);
			t4.addCondition(COND::MAX_FOOD_USED, 115);
			t4.addCondition(COND::MAX_TIME, 19999);
			t4.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, bot->locH->bases[1].getRallyPoint(), 30.0F);
			t4.addCondition(COND::MAX_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_immortal.addTrigger(t4);
			Trigger t5(bot);  // not zerg, have robotics bay, have colossus
			t5.addCondition(COND::MIN_MINERALS, 275);
			t5.addCondition(COND::MIN_GAS, 100);
			t5.addCondition(COND::MIN_FOOD, 4);
			t5.addCondition(COND::ENEMY_RACE_ZERG, 0, false);
			t5.addCondition(COND::MAX_FOOD_USED, 115);
			t5.addCondition(COND::MAX_TIME, 19999);
			t5.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, bot->locH->bases[1].getRallyPoint(), 30.0F);
			t5.addCondition(COND::MAX_UNIT_OF_TYPE, 6, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_immortal.addTrigger(t5);
			Trigger t6(bot); // zerg, have robotics bay, have colossus
			t6.addCondition(COND::MIN_MINERALS, 350);
			t6.addCondition(COND::MIN_GAS, 150);
			t6.addCondition(COND::MIN_FOOD, 4);
			t6.addCondition(COND::ENEMY_RACE_ZERG);
			t6.addCondition(COND::MAX_FOOD_USED, 115);
			t6.addCondition(COND::MAX_TIME, 19999);
			t6.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, bot->locH->bases[1].getRallyPoint(), 30.0F);
			t6.addCondition(COND::MAX_UNIT_OF_TYPE, 5, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
			t6.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t6.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_immortal.addTrigger(t6);
			bot->addStrat(train_immortal);
		}
		{	// train an observer
			Precept train_observer(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, sc2::ABILITY_ID::TRAIN_OBSERVER);
			d.allowMultiple();
			train_observer.addDirective(d);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 25);
			t.addCondition(COND::MIN_GAS, 75);
			t.addCondition(COND::MIN_FOOD, 1);
			t.addCondition(COND::MIN_FOOD_USED, 100);
			t.addCondition(COND::ENEMY_RACE_PROTOSS); // only really want for dark templars
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_OBSERVER);
			t.addCondition(COND::MAX_UNITS_USING_ABILITY, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, sc2::ABILITY_ID::TRAIN_OBSERVER);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_observer.addTrigger(t);
			bot->addStrat(train_observer);
		}


		// pre-create a container of TriggerConditions to reduce copy/pasting same conditions over and over
		// all of these being met means the initial buildings are intact, so we won't create a building in their place

		std::vector<Trigger::TriggerCondition> startup_base_conds;
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 12.0F));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE));

		{	// train collosus
			Precept train_colossus(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, sc2::ABILITY_ID::TRAIN_COLOSSUS);
			d.allowMultiple();
			train_colossus.addDirective(d);
			Trigger t(bot);  // not zerg
			t.addCondition(COND::MIN_MINERALS, 300);
			t.addCondition(COND::MIN_GAS, 200);
			t.addCondition(COND::MIN_FOOD, 6);
			t.addCondition(COND::ENEMY_RACE_ZERG, 0, false);
			t.addCondition(COND::MAX_FOOD_USED, 149);
			t.addCondition(COND::MAX_TIME, 19999);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_colossus.addTrigger(t);
			Trigger t2(bot); // zerg
			t2.addCondition(COND::MIN_MINERALS, 300);
			t2.addCondition(COND::MIN_GAS, 200);
			t2.addCondition(COND::MIN_FOOD, 6);
			t2.addCondition(COND::ENEMY_RACE_ZERG);
			t2.addCondition(COND::MAX_FOOD_USED, 149);
			t2.addCondition(COND::MAX_TIME, 19999);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_colossus.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MIN_MINERALS, 300);
			t3.addCondition(COND::MIN_GAS, 200);
			t3.addCondition(COND::MIN_FOOD, 6);
			t3.addCondition(COND::MIN_FOOD_USED, 149);
			t3.addCondition(COND::MAX_TIME, 19999);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
			train_colossus.addTrigger(t3);
			bot->addStrat(train_colossus);
		}
		{	// build assimilators at main base
			Precept assim_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[0].getTownhall());
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_MINERALS, 75);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[0].getTownhall(), 10.0F);
			assim_1.addDirective(d);
			assim_1.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 75);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[0].getTownhall(), 10.0F);
			assim_1.addTrigger(t2);
			bot->addStrat(assim_1);
		}
		{	// build pylon near cannons
			Precept defense_pylon(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[1].getDefendPoint(0), 2.0F);
			Trigger t(bot);
			for (auto tc : startup_base_conds)
				t.addCondition(tc);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
			t.addCondition(COND::MAX_FOOD, 6);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[1].getDefendPoint(0), 2.5F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			defense_pylon.addDirective(d);
			defense_pylon.addTrigger(t);
			bot->addStrat(defense_pylon);
		}
		{	// build more cannons at our defense point
			Precept more_cannons(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->getStoredLocation("CANNON_1"), 4.0F);
			//d.allowMultiple();
			Trigger t(bot);
			for (auto tc : startup_base_conds)
				t.addCondition(tc);
			t.addCondition(COND::MIN_MINERALS, 175);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 2, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[1].getDefendPoint(0), 12.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].getDefendPoint(0), 8.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 2, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].getDefendPoint(0), 6.0F);
			more_cannons.addDirective(d);
			more_cannons.addTrigger(t);
			bot->addStrat(more_cannons);
		}
		{	// handle training stalkers
			Precept train_stalker(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_STALKER);
			d.allowMultiple();
			Trigger t(bot); // get 2 stalkers out asap
			t.addCondition(COND::MIN_MINERALS, 125);
			t.addCondition(COND::MIN_GAS, 50);
			t.addCondition(COND::MIN_FOOD, 2);
			t.addCondition(COND::MAX_FOOD_USED, 120);
			t.addCondition(COND::MAX_TIME, 19999);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 2, sc2::UNIT_TYPEID::PROTOSS_STALKER, bot->locH->bases[1].getRallyPoint());
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			train_stalker.addDirective(d);
			train_stalker.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 300);
			t2.addCondition(COND::MIN_GAS, 100);
			t2.addCondition(COND::MIN_FOOD, 2);
			t2.addCondition(COND::MAX_FOOD_CAP, 100);
			t2.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // enemy is not zerg, avoid too many stalkers
			t2.addCondition(COND::MAX_FOOD_USED, 120);
			t2.addCondition(COND::MAX_TIME, 19999);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 12, sc2::UNIT_TYPEID::PROTOSS_STALKER, bot->locH->bases[1].getRallyPoint());
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_ZEALOT);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			train_stalker.addDirective(d);
			train_stalker.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MIN_MINERALS, 400);
			t3.addCondition(COND::MIN_GAS, 400);
			t3.addCondition(COND::MIN_FOOD, 2);
			t3.addCondition(COND::MAX_FOOD_CAP, 100);
			t3.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // enemy is not zerg, avoid too many stalkers
			t3.addCondition(COND::MAX_FOOD_USED, 120);
			t3.addCondition(COND::MAX_TIME, 19999);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 12, sc2::UNIT_TYPEID::PROTOSS_STALKER, bot->locH->bases[1].getRallyPoint());
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			train_stalker.addTrigger(t3);
			Trigger t4(bot);
			t4.addCondition(COND::MIN_MINERALS, 400);
			t4.addCondition(COND::MIN_GAS, 350);
			t4.addCondition(COND::MIN_FOOD, 2);
			t4.addCondition(COND::MIN_FOOD_CAP, 101);
			t4.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // enemy is not zerg, avoid too many stalkers
			t4.addCondition(COND::MAX_FOOD_USED, 120);
			t4.addCondition(COND::MAX_TIME, 19999);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
			t4.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 12, sc2::UNIT_TYPEID::PROTOSS_STALKER, bot->locH->bases[1].getRallyPoint());
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			train_stalker.addTrigger(t4);
			Trigger t5(bot);
			t5.addCondition(COND::MIN_MINERALS, 400);
			t5.addCondition(COND::MIN_GAS, 350);
			t5.addCondition(COND::MIN_FOOD, 2);
			t5.addCondition(COND::ENEMY_RACE_ZERG); // enemy race is zerg, build tons
			t5.addCondition(COND::MAX_FOOD_USED, 120);
			t5.addCondition(COND::MAX_TIME, 19999);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE, 5, sc2::UNIT_TYPEID::PROTOSS_ZEALOT);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			train_stalker.addTrigger(t5);
			Trigger t6(bot);
			t6.addCondition(COND::MIN_MINERALS, 125);
			t6.addCondition(COND::MIN_GAS, 50);
			t6.addCondition(COND::MIN_FOOD, 2);
			t6.addCondition(COND::ENEMY_RACE_ZERG); // enemy race is zerg, build tons
			t6.addCondition(COND::MAX_FOOD_USED, 120);
			t6.addCondition(COND::MAX_TIME, 19999);
			t6.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, bot->locH->bases[1].getRallyPoint());
			t6.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 20, sc2::UNIT_TYPEID::PROTOSS_STALKER, bot->locH->bases[1].getRallyPoint());
			t6.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t6.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			train_stalker.addTrigger(t6);
			Trigger t7(bot);
			t7.addCondition(COND::MIN_MINERALS, 290);
			t7.addCondition(COND::MIN_GAS, 50);
			t7.addCondition(COND::MIN_FOOD, 2);
			t7.addCondition(COND::ENEMY_RACE_ZERG); // enemy race is zerg, build tons
			t7.addCondition(COND::MAX_FOOD_USED, 120);
			t7.addCondition(COND::MAX_TIME, 19999);
			t7.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t7.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t7.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			train_stalker.addTrigger(t7);
			bot->addStrat(train_stalker);
		}
		{	// handle training sentries
			Precept train_sentry(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_SENTRY);
			d.allowMultiple();
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 110);
			t.addCondition(COND::MIN_GAS, 100);
			t.addCondition(COND::MIN_FOOD, 2);
			t.addCondition(COND::MAX_FOOD_USED, 109);
			t.addCondition(COND::MAX_TIME, 19999);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			t.addCondition(COND::MAX_UNITS_USING_ABILITY, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_SENTRY);
			train_sentry.addDirective(d);
			train_sentry.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 200);
			t2.addCondition(COND::MIN_GAS, 100);
			t2.addCondition(COND::MIN_FOOD, 2);
			t2.addCondition(COND::MAX_FOOD_USED, 109);
			t2.addCondition(COND::MAX_TIME, 19999);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_STALKER);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			t2.addCondition(COND::MAX_UNITS_USING_ABILITY, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_SENTRY);
			train_sentry.addTrigger(t2);
			bot->addStrat(train_sentry);
		}
		{	// use a forcefield at the base entry when units are swarming in. Ideally it will trap one or two inside.
			Precept force_field(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[1].getRallyPoint(), Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_SENTRY, sc2::ABILITY_ID::EFFECT_FORCEFIELD, bot->getStoredLocation("FORCE_FIELD"), 8.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_SENTRY);
			t.addCondition(COND::THREAT_EXISTS_NEAR_LOCATION, bot->getStoredLocation("FORCE_FIELD"), 12.0F);
			t.addCondition(COND::MIN_ENEMY_UNITS_NEAR_LOCATION, 1, bot->locH->bases[1].getRallyPoint(), 7.0F);
			t.addCondition(COND::MAX_NEUTRAL_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::NEUTRAL_FORCEFIELD);
			force_field.addTrigger(t);
			force_field.addDirective(d);
			bot->addStrat(force_field);
		}
		{
			Precept hallucination(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_SENTRY, sc2::ABILITY_ID::HALLUCINATION_COLOSSUS);
			Trigger t(bot);
			t.addCondition(COND::TIMER_1_MIN_STEPS_PAST, 850);
			hallucination.addTrigger(t);
			hallucination.addDirective(d);
			bot->addStrat(hallucination);
		}
		{	// handle training zealots
			Precept train_zealot(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_ZEALOT);
			d.allowMultiple();
			Trigger t(bot); // always build zealots if gas is low
			t.addCondition(COND::MIN_MINERALS, 125);
			t.addCondition(COND::MAX_GAS, 49);
			t.addCondition(COND::MIN_FOOD, 2);
			//t.addCondition(COND::MAX_UNIT_OF_TYPE, 15, sc2::UNIT_TYPEID::PROTOSS_ZEALOT);
			train_zealot.addDirective(d);
			train_zealot.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 320); // give colossus a chance to build
			t2.addCondition(COND::MIN_GAS, 50);
			t2.addCondition(COND::MIN_FOOD, 2);
			t2.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // enemy race is not zerg
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_STALKER);
			train_zealot.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MIN_MINERALS, 320); // give colossus a chance to build
			t3.addCondition(COND::MIN_GAS, 50);
			t3.addCondition(COND::MIN_FOOD, 4);
			t3.addCondition(COND::ENEMY_RACE_ZERG); // enemy race is zerg
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 6, sc2::UNIT_TYPEID::PROTOSS_STALKER);
			train_zealot.addTrigger(t3);
			Trigger t4(bot);
			t4.addCondition(COND::MIN_MINERALS, 320); // give colossus a chance to build
			t4.addCondition(COND::MIN_FOOD, 2);
			t4.addCondition(COND::ENEMY_RACE_ZERG, 0, false); // enemy race is not zerg
			t4.addCondition(COND::MIN_FOOD_CAP, 91);
			train_zealot.addTrigger(t4);
			Trigger t5(bot);
			t5.addCondition(COND::MIN_MINERALS, 400); // remove stalker requirement but increase mineral requirement
			t5.addCondition(COND::MIN_GAS, 50);
			t5.addCondition(COND::MIN_FOOD, 2);
			t5.addCondition(COND::ENEMY_RACE_ZERG); // enemy race is zerg
			train_zealot.addTrigger(t5);
			bot->addStrat(train_zealot);
		}
		{	// build twilight council at main base build area 0
			Precept twilight_council(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_TWILIGHTCOUNCIL, bot->locH->bases[0].getBuildArea(0), 14.0F);
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 0, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			twilight_council.addDirective(d);
			twilight_council.addTrigger(t);
			bot->addStrat(twilight_council);
		}
		{	// research extended lance for collossus to outrange defenses
			Precept research_lance(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY, sc2::ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::EXTENDEDTHERMALLANCE, false);
			research_lance.addDirective(d);
			research_lance.addTrigger(t);
			bot->addStrat(research_lance);
		}
		{	// research charge for zealots
			Precept research_charge(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, sc2::ABILITY_ID::RESEARCH_CHARGE);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_GAS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			//t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::BLINKTECH);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::CHARGE, false);
			research_charge.addDirective(d);
			research_charge.addTrigger(t);
			bot->addStrat(research_charge);
		}
		{	// research blink for stalkers (they will automatically blink away when taking damage that puts their health low)
			Precept research_blink(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, sc2::ABILITY_ID::RESEARCH_BLINK);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::CHARGE);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::BLINKTECH, false);
			research_blink.addDirective(d);
			research_blink.addTrigger(t);
			bot->addStrat(research_blink);
		}
		{	// build assimilators at expansion
			Precept assim_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[1].getTownhall());
			d.allowMultiple();
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 75);
			t.addCondition(COND::MAX_GAS, 300);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall(), 1.5F);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[1].getTownhall());
			assim_2.addDirective(d);
			assim_2.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 75);
			t2.addCondition(COND::MAX_GAS, 300);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall(), 1.5F);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[1].getTownhall());
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[1].getTownhall());
			assim_2.addTrigger(t2);
			bot->addStrat(assim_2);
		}
		{	// continuously upgrade attack at forge when possible
			Precept upgrade_attack(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_FORGE, sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_GAS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1, false);
			upgrade_attack.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_GAS, 150);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			t2.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1);
			t2.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2, false);
			upgrade_attack.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MIN_MINERALS, 200);
			t3.addCondition(COND::MIN_GAS, 200);
			t3.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2);
			t3.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3, false);
			upgrade_attack.addTrigger(t3);
			upgrade_attack.addDirective(d);
			bot->addStrat(upgrade_attack);
		}
		{	// upgrade shields at forge when attack is full upgraded
			// DISABLED this because the game ends before it would ever be worth it.
			Precept upgrade_shields(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_FORGE, sc2::ABILITY_ID::RESEARCH_PROTOSSSHIELDS);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL1, false);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3, true);
			upgrade_shields.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 225);
			t2.addCondition(COND::MIN_GAS, 225);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			t2.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL1);
			t2.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL2, false);
			upgrade_shields.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MIN_MINERALS, 200);
			t3.addCondition(COND::MIN_GAS, 200);
			t3.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL2);
			t3.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL3, false);
			upgrade_shields.addTrigger(t3);
			upgrade_shields.addDirective(d);
			//bot->addStrat(upgrade_shields);
		}
		{	// handle our nexus using chronoboost
			Precept use_chrono(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, bot->locH->bases[0].getTownhall());
			Trigger t(bot);
			t.addCondition(COND::MIN_FOOD_CAP, 16);
			t.addCondition(COND::HAS_ABILITY_READY, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST);
			use_chrono.addDirective(d);
			use_chrono.addTrigger(t);
			bot->addStrat(use_chrono);
		}
		{	// send our scout to the highest threat locations (pre-defined as the enemy start locations)
			Precept scout_bases(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_SCOUT}, sc2::ABILITY_ID::ATTACK, bot->getStoredLocation("CANNON_1"), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->getHighestThreatLocation(); };
			d.setTargetLocationFunction(this, bot, func);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 1, std::unordered_set<FLAGS>{FLAGS::IS_SCOUT});
			scout_bases.addDirective(d);
			scout_bases.addTrigger(t);
			bot->addStrat(scout_bases);
		}
		{	// set our first unit (a zealot) to be a scout
			Precept set_scout(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::SET_FLAG, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, FLAGS::IS_SCOUT);
			Trigger t(bot);
			t.addCondition(COND::MAX_DEAD_MOBS, 0);
			t.addCondition(COND::MAX_UNIT_WITH_FLAGS, 0, std::unordered_set<FLAGS>{FLAGS::IS_SCOUT});
			set_scout.addDirective(d);
			set_scout.addTrigger(t);
			bot->addStrat(set_scout);
		}
		{	// set all army units to use the smartStayHomeAndDefend() function until it is time to attack
			Precept defend_home(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->locH->bases[1].getRallyPoint(), 2.5F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->smartStayHomeAndDefend(); };
			d.setTargetLocationFunction(this, bot, func);
			d.setIgnoreDistance(2.5F);
			d.excludeFlag(FLAGS::IS_SCOUT);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 1, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			t.addCondition(COND::TIMER_1_SET, 0, false);
			t.addCondition(COND::MAX_TIME, 19999);
			defend_home.addTrigger(t);
			defend_home.addDirective(d);
			bot->addStrat(defend_home);
		}


		{	// designate 4 probes to not defend at all times, so our economy still runs
			Precept assign_non_defender(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::ACTION_TYPE::SET_FLAG, sc2::UNIT_TYPEID::PROTOSS_PROBE, FLAGS::NON_DEFENDER, 20.0F);
			Trigger t(bot);
			t.addCondition(COND::MAX_UNIT_WITH_FLAGS, 4, std::unordered_set<FLAGS>{FLAGS::NON_DEFENDER});
			assign_non_defender.addTrigger(t);
			assign_non_defender.addDirective(d);
			bot->addStrat(assign_non_defender);
		}

		{	// set timer to send workers to defend expansion when army is insufficient
			Precept workers_defend_init_timer(bot);
			Directive d(Directive::GAME_VARIABLES, Directive::SET_TIMER_2, 0);
			Trigger t(bot);
			t.addCondition(COND::MIN_ENEMY_UNITS_NEAR_LOCATION, 2, bot->locH->bases[1].getTownhall(), 15.0F);
			t.addCondition(COND::MAX_UNIT_WITH_FLAGS_NEAR_LOCATION, 3, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, bot->locH->bases[1].getRallyPoint(), 50.0F);
			workers_defend_init_timer.addDirective(d);
			workers_defend_init_timer.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::THREAT_EXISTS_NEAR_LOCATION, bot->locH->bases[1].getRallyPoint(), 12.0F);
			t2.addCondition(COND::MAX_TIME, 3999);
			t2.addCondition(COND::MIN_ENEMY_UNITS_NEAR_LOCATION, 1, bot->locH->bases[1].getRallyPoint(), 12.0F);
			workers_defend_init_timer.addTrigger(t2);
			bot->addStrat(workers_defend_init_timer);
		}
		{	// reset the defense timer once enemies are cleared or defense army is sufficient
			Precept reset_worker_defense_timer(bot);
			Directive d(Directive::GAME_VARIABLES, Directive::RESET_TIMER_2);
			Trigger t(bot);
			t.addCondition(COND::MAX_ENEMY_UNITS_NEAR_LOCATION, 0, bot->locH->bases[1].getTownhall(), 15.0F);
			t.addCondition(COND::MIN_TIME, 4000);
			reset_worker_defense_timer.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_UNIT_WITH_FLAGS_NEAR_LOCATION, 6, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, bot->locH->bases[1].getRallyPoint(), 50.0F);
			reset_worker_defense_timer.addTrigger(t2);
			reset_worker_defense_timer.addDirective(d);
			Trigger t3(bot);
			t3.addCondition(COND::MAX_TIME, 3999);
			t3.addCondition(COND::MAX_ENEMY_UNITS_NEAR_LOCATION, 0, bot->locH->bases[1].getRallyPoint(), 12.0F);
			reset_worker_defense_timer.addTrigger(t3);
			bot->addStrat(reset_worker_defense_timer);
		}
		{	// send workers to defend expansion when defense timer is initialized
			Precept workers_defend_expansion(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_WORKER}, sc2::ABILITY_ID::ATTACK, bot->locH->bases[1].getRallyPoint(), 3.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->smartStayHomeAndDefend(); };
			d.setTargetLocationFunction(this, bot, func);
			d.excludeFlag(FLAGS::NON_DEFENDER);
			t.addCondition(COND::TIMER_2_SET, 0, true);
			//t.addCondition(COND::MAX_UNIT_WITH_FLAGS_NEAR_LOCATION, 3, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, bot->locH->bases[1].getTownhall(), 16.0F);
			workers_defend_expansion.addTrigger(t);
			workers_defend_expansion.addDirective(d);
			bot->addStrat(workers_defend_expansion);
		}

		// previously sent all workers - but often let to wrecking our economy when unnecessary
		/*
		{	// send workers to defend expansion when defense timer is initialized
			Precept workers_defend_expansion(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_WORKER}, sc2::ABILITY_ID::ATTACK, bot->locH->bases[1].getRallyPoint(), 3.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->smartStayHomeAndDefend(); };
			d.setTargetLocationFunction(this, bot, func);
			t.addCondition(COND::TIMER_2_SET, 0, true);
			//t.addCondition(COND::MAX_UNIT_WITH_FLAGS_NEAR_LOCATION, 3, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, bot->locH->bases[1].getTownhall(), 16.0F);
			workers_defend_expansion.addTrigger(t);
			workers_defend_expansion.addDirective(d);
			bot->addStrat(workers_defend_expansion);
		} */
		{	// initialize the timer for grouping up at a rally point en route to attacking
			Precept init_group_timer(bot);
			Directive d(Directive::GAME_VARIABLES, Directive::ACTION_TYPE::SET_TIMER_1, 0);
			Trigger t(bot);
			t.addCondition(COND::TIMER_1_SET, 0, false);
			//t.addCondition(COND::MIN_FOOD_CAP, 105);
			t.addCondition(COND::MIN_FOOD_USED, 100);
			t.addCondition(COND::MAX_TIME, 15499);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t.addCondition(COND::THREAT_EXISTS_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), 50.0F, false);
			init_group_timer.addDirective(d);
			init_group_timer.addTrigger(t);
			Trigger t2(bot); // require a larger army for second push
			t2.addCondition(COND::TIMER_1_SET, 0, false);
			t2.addCondition(COND::MIN_FOOD_USED, 120);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t2.addCondition(COND::THREAT_EXISTS_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), 50.0F, false);
			init_group_timer.addTrigger(t2);
			Trigger t3(bot);  // if our economy is f***ed, go all in
			t3.addCondition(COND::TIMER_1_SET, 0, false);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_PROBE);
			t3.addCondition(COND::MAX_MINERALS, 49);
			init_group_timer.addTrigger(t3);
			bot->addStrat(init_group_timer);
		}


		{	// send all army units to the pre-group area outside of the base before sending the attack
			Precept group_at_pre_pre_prep_area(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->locH->bases[3].getDefendPoint(1), 4.0F);
			Trigger t(bot);
			d.setIgnoreDistance(7.0F);
			d.setOverrideOther(); // grab mobs who were on their way to attack the enemy
			//t.addCondition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t.addCondition(COND::TIMER_1_MIN_STEPS_PAST, 0);
			t.addCondition(COND::TIMER_1_MAX_STEPS_PAST, 169);
			//t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 12, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			group_at_pre_pre_prep_area.addDirective(d);
			group_at_pre_pre_prep_area.addTrigger(t);
			bot->addStrat(group_at_pre_pre_prep_area);
		}

		// removing this extra grouping spot as it often caused the army to split up
		/* 
		{	// send all army units to the pre-group area outside of the base before sending the attack
			Precept group_at_pre_prep_area(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->locH->bases[3].getDefendPoint(1), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->getRallyPointBeforeRallyPoint(); };
			d.setTargetLocationFunction(this, bot, func);
			d.setIgnoreDistance(7.0F);
			d.setOverrideOther(); // grab mobs who were on their way to attack the enemy
			//t.addCondition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t.addCondition(COND::TIMER_1_MIN_STEPS_PAST, 100);
			t.addCondition(COND::TIMER_1_MAX_STEPS_PAST, 379);
			//t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 12, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			group_at_pre_prep_area.addDirective(d);
			group_at_pre_prep_area.addTrigger(t);
			bot->addStrat(group_at_pre_prep_area);
		} */
		{	// send all army units to group at a rally point en route to the attack location and wait for 850 gameloop steps
			Precept group_at_prep_area(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->locH->getCenterPathableLocation(), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->getRallyPointTowardsThreat(); };
			d.setTargetLocationFunction(this, bot, func);
			d.setIgnoreDistance(7.0F);
			d.setOverrideOther(); // grab mobs who were on their way to attack the enemy
			t.addCondition(COND::TIMER_1_MIN_STEPS_PAST, 170);
			t.addCondition(COND::TIMER_1_MAX_STEPS_PAST, 849);
			group_at_prep_area.addDirective(d);
			group_at_prep_area.addTrigger(t);
			bot->addStrat(group_at_prep_area);
		}


		/*			This was the original but testing new grouping for now - also changed to 650 instead of 750
		{	// send all army units to group at a rally point en route to the attack location and wait for 750 gameloop steps
			Precept group_at_prep_area(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->locH->getCenterPathableLocation(), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->getRallyPointTowardsThreat(); };
			d.setTargetLocationFunction(this, bot, func);
			d.setIgnoreDistance(7.0F);
			d.setOverrideOther(); // grab mobs who were on their way to attack the enemy
			//t.addCondition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
			t.addCondition(COND::TIMER_1_MIN_STEPS_PAST, 0);
			t.addCondition(COND::TIMER_1_MAX_STEPS_PAST, 719);
			//t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 12, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			group_at_prep_area.addDirective(d);
			group_at_prep_area.addTrigger(t);
			bot->addStrat(group_at_prep_area);
		} */
		{	// after timer passes 750 gameloop steps, send in the attack 
			Precept attack_and_explore(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, sc2::Point2D(1, 1), 4.0F);
			Trigger t(bot);
			bot->storeInt("ATTACK_DIR_ID", d.getID()); // identifier use to determine when first attack was launched
			d.excludeFlag(FLAGS::IS_FLYING);
			d.setContinuous();
			auto func = [this]() { return bot->locH->smartPriorityAttack(); };
			d.setTargetLocationFunction(this, bot, func);
			attack_and_explore.addDirective(d);
			t.addCondition(COND::TIMER_1_MIN_STEPS_PAST, 850);
			attack_and_explore.addTrigger(t);
			bot->addStrat(attack_and_explore);
		}
		{	// handle flyers during attack
			Precept attack_and_explore_flying(bot); 
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_FLYING}, sc2::ABILITY_ID::ATTACK, sc2::Point2D(1, 1), 4.0F);
			Trigger t(bot);
			d.setContinuous();
			auto func = [this]() { return bot->locH->getAttackingForceLocation(); }; //flyers stay with the army instead of taking shortcuts
			d.setTargetLocationFunction(this, bot, func);
			attack_and_explore_flying.addDirective(d);
			t.addCondition(COND::TIMER_1_MIN_STEPS_PAST, 850);
			t.addCondition(COND::MAX_TIME, 21999);
			attack_and_explore_flying.addTrigger(t);
			bot->addStrat(attack_and_explore_flying);
		}
		{	// after 750 gameloop steps, send in the attack 
			Precept attack_and_explore_late(bot); // if greater than 22,000 steps, flyers go their own way
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, sc2::Point2D(1, 1), 4.0F);
			Trigger t(bot);
			d.excludeFlag(FLAGS::IS_FLYING);
			d.setContinuous();
			auto func = [this]() { return bot->locH->smartPriorityAttack(); };
			d.setTargetLocationFunction(this, bot, func);
			attack_and_explore_late.addDirective(d);
			t.addCondition(COND::MIN_TIME, 22000);
			attack_and_explore_late.addTrigger(t);
			bot->addStrat(attack_and_explore_late);
		}
		{	// if food usage drops below 90, stop sending units to join the attack, and build up another force first
			Precept reset_group_timer(bot);
			Directive d(Directive::GAME_VARIABLES, Directive::ACTION_TYPE::RESET_TIMER_1);
			Trigger t(bot);
			t.addCondition(COND::TIMER_1_SET);
			t.addCondition(COND::MAX_FOOD_USED, 90);
			reset_group_timer.addDirective(d);
			reset_group_timer.addTrigger(t);
			bot->addStrat(reset_group_timer);
		}
		{	// in case things don't end, start building up stuff to clean up
			Precept stargate_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_STARGATE, bot->locH->bases[0].getBuildArea(2));
			Trigger t(bot);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_FOOD_USED, 130);
			t.addCondition(COND::MIN_FOOD_USED, 50);
			t.addCondition(COND::MIN_TIME, 17000);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			stargate_1.addTrigger(t);
			stargate_1.addDirective(d);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_GAS, 150);
			t2.addCondition(COND::MIN_TIME, 20000);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			stargate_1.addTrigger(t2);
			bot->addStrat(stargate_1);
		}

		{	// train voidrays for cleanup
			Precept train_void(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_STARGATE, sc2::ABILITY_ID::TRAIN_VOIDRAY);
			d.allowMultiple(); // more than one stargate can train at the same time
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 250);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::MIN_FOOD_USED, 110);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			train_void.addDirective(d);
			train_void.addTrigger(t);
			Trigger t2(bot); // after 20000 steps, train voids regardless of unit count
			t2.addCondition(COND::MIN_MINERALS, 250);
			t2.addCondition(COND::MIN_GAS, 150);
			t2.addCondition(COND::MIN_FOOD, 4);
			t2.addCondition(COND::MIN_TIME, 20000);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			train_void.addTrigger(t2);
			bot->addStrat(train_void);
		}
		{	// void rays clean up
			Precept flyers_search(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_FLYING}, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation(), 4.0F);
			auto func = [this]() { return bot->locH->smartAttackFlyingLocation(); };
			d.setTargetLocationFunction(this, bot, func);
			Trigger t(bot); // after 22,000 timesteps, send flyers to search non-pathable areas
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 1, std::unordered_set<FLAGS>{FLAGS::IS_FLYING});
			t.addCondition(COND::MIN_TIME, 22000);
			flyers_search.addDirective(d);
			flyers_search.addTrigger(t);
			bot->addStrat(flyers_search);
		}

	}
	else {
		// for the 1v1 maps, a simple 4 gateway proxy strategy should do the trick

		bot->storeUnitType("_CHRONOBOOST_TARGET", sc2::UNIT_TYPEID::PROTOSS_GATEWAY);   // special tag to specify a unit that chronoboost will only target
		if (map_index == 3) { // Proxima
			if (p_id == 1) { // top
				bot->storeLocation("PROXY_INITIAL_LOC", sc2::Point2D(45.0, 103.0));
				bot->storeLocation("DECOY_LOC", sc2::Point2D(78.0, 96.0));
			}
			if (p_id == 2) { // bottom
				bot->storeLocation("PROXY_INITIAL_LOC", sc2::Point2D(155.0, 65.0));
				bot->storeLocation("DECOY_LOC", sc2::Point2D(122.0, 72.0));
			}
		}
		if (map_index == 2) { // Bel'Shir
			if (p_id == 1) { // bottom
				bot->storeLocation("PROXY_INITIAL_LOC", sc2::Point2D(23.0, 38.0));
				bot->storeLocation("DECOY_LOC", sc2::Point2D(62.0, 132.0));
			}
			if (p_id == 2) { // top
				bot->storeLocation("PROXY_INITIAL_LOC", sc2::Point2D(121.0, 122.0));
				bot->storeLocation("DECOY_LOC", sc2::Point2D(82.0, 28.0));
			}
		}

		{	// send 1 probe to the proxy point
			Precept assign_proxy(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->getStoredLocation("PROXY_INITIAL_LOC"), Directive::ACTION_TYPE::SET_FLAG, sc2::UNIT_TYPEID::PROTOSS_PROBE, FLAGS::IS_PROXY, 4.0F);
			assign_proxy.addDirective(d);
			Trigger t(bot);
			t.addCondition(COND::MAX_UNIT_WITH_FLAGS, 0, std::unordered_set<FLAGS>{FLAGS::IS_PROXY});
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->getStoredLocation("PROXY_INITIAL_LOC"), 6.0F);
			assign_proxy.addTrigger(t);
			bot->addStrat(assign_proxy);
		}
		{	// send the proxy probe in with the first wave of zealots
			Precept send_decoy_home(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->getStoredLocation("DECOY_LOC"), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::ATTACK, bot->locH->bases[0].getTownhall(), 6.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->getStoredLocation("DECOY_LOC"), 4.0F);
			send_decoy_home.addDirective(d);
			send_decoy_home.addTrigger(t);
			bot->addStrat(send_decoy_home);
		}
		{
			Precept disable_proxy_harvesting(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::DISABLE_DEFAULT_DIRECTIVE, std::unordered_set<FLAGS>{FLAGS::IS_PROXY});
			Trigger t(bot);
			d.excludeFlag(FLAGS::DEF_DIR_DISABLED);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 1, std::unordered_set<FLAGS>{FLAGS::IS_PROXY});
			disable_proxy_harvesting.addDirective(d);
			disable_proxy_harvesting.addTrigger(t);
			bot->addStrat(disable_proxy_harvesting);
		}
		{
			Precept send_proxy(bot);
			Directive d(Directive::MATCH_FLAGS_NEAR_LOCATION, Directive::EXACT_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_PROXY}, sc2::ABILITY_ID::ATTACK, bot->getStoredLocation("PROXY_INITIAL_LOC"), bot->locH->getProxyLocation(), 4.0F, 2.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 1, std::unordered_set<FLAGS>{FLAGS::IS_PROXY});
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->getProxyLocation(), 8.0F);
			send_proxy.addTrigger(t);
			send_proxy.addDirective(d);
			bot->addStrat(send_proxy);
		}
		{	// build probes at nexus, but only a minimal amount
			Precept base_probe(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			base_probe.addDirective(d);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 50);
			t.addCondition(COND::MIN_FOOD, 1);
			t.addCondition(COND::MAX_TIME, 2650);
			t.addCondition(COND::MAX_UNITS_USING_ABILITY, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 13, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[0].getTownhall(), 24.0f);
			base_probe.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_TIME, 12000);
			t2.addCondition(COND::MIN_MINERALS, 50);
			t2.addCondition(COND::MIN_FOOD, 1);
			t2.addCondition(COND::MAX_UNITS_USING_ABILITY, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 17, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[0].getTownhall(), 18.0f);
			base_probe.addTrigger(t2);
			bot->addStrat(base_probe);
		}
		{	// build proxy pylon
			Precept proxy_pylon(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->getProxyLocation(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->getProxyLocation(), 15.0F, 4.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->getProxyLocation(), 30.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->getProxyLocation());
			proxy_pylon.addDirective(d);
			proxy_pylon.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 100);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->getProxyLocation());
			proxy_pylon.addTrigger(t2);
			bot->addStrat(proxy_pylon);
		}
		{	// build exactly 4 proxy gateways
			Precept proxy_gateway(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->getProxyLocation(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->locH->getProxyLocation());
			Trigger t(bot); // handle case where 3 are built and none are under construction
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->getProxyLocation(), 30.0F);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->getProxyLocation());
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->locH->getProxyLocation());
			proxy_gateway.addDirective(d);
			proxy_gateway.addTrigger(t);
			Trigger t2(bot); // handle case where 2 are built and 1 is under construction
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->getProxyLocation());
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->getProxyLocation(), 30.0F);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->locH->getProxyLocation());
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			proxy_gateway.addTrigger(t2);
			Trigger t3(bot); // handle case where 1 is built and 2 are under construction
			t3.addCondition(COND::MIN_MINERALS, 150);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->getProxyLocation());
			t3.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->getProxyLocation(), 30.0F);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->locH->getProxyLocation());
			t3.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			proxy_gateway.addTrigger(t3);
			Trigger t4(bot); // handle case where none are built and 3 are under construction
			t4.addCondition(COND::MIN_MINERALS, 150);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->getProxyLocation());
			t4.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->getProxyLocation(), 30.0F);
			t4.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->locH->getProxyLocation());
			t4.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			proxy_gateway.addTrigger(t4);
			bot->addStrat(proxy_gateway);
		}
		{	// use chronoboost on gateways
			Precept use_chrono(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, bot->locH->bases[0].getTownhall());
			d.allowMultiple();
			Trigger t(bot);
			t.addCondition(COND::HAS_ABILITY_READY, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 2, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}); // don't chronoboost out units that will just be waiting
			use_chrono.addDirective(d);
			use_chrono.addTrigger(t);
			bot->addStrat(use_chrono);
		}
		{	// train zealots at proxy point
			Precept train_zealot(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_ZEALOT);
			d.allowMultiple(); // more than one gateway can train at the same time
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_FOOD, 2);
			t.addCondition(COND::MAX_FOOD_USED, 36);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			train_zealot.addDirective(d);
			train_zealot.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 100);
			t2.addCondition(COND::MIN_FOOD, 2);
			t2.addCondition(COND::MAX_FOOD_USED, 36);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			train_zealot.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MIN_MINERALS, 100);
			t3.addCondition(COND::MIN_FOOD, 2);
			t3.addCondition(COND::MAX_FOOD_USED, 36);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			train_zealot.addTrigger(t3);
			Trigger t4(bot);
			t4.addCondition(COND::MIN_MINERALS, 100);
			t4.addCondition(COND::MIN_FOOD, 2);
			t4.addCondition(COND::MAX_FOOD_USED, 36);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			train_zealot.addTrigger(t4);
			Trigger t5(bot);
			t5.addCondition(COND::MIN_MINERALS, 100);
			t5.addCondition(COND::MIN_FOOD, 2);
			t5.addCondition(COND::MAX_FOOD_USED, 36);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t5.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			train_zealot.addTrigger(t5);
			Trigger t6(bot);
			t6.addCondition(COND::MIN_MINERALS, 100);
			t6.addCondition(COND::MIN_FOOD, 2);
			t6.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_VOIDRAY);
			train_zealot.addTrigger(t6);
			bot->addStrat(train_zealot);
		}
		{	// send the proxy probe in with the first wave of zealots
			Precept send_probe_attack(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->getProxyLocation(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation(), 20.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_ZEALOT);
			send_probe_attack.addDirective(d);
			send_probe_attack.addTrigger(t);
			bot->addStrat(send_probe_attack);
		}
		{	// build more pylons if the game continues
			Precept more_pylons(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].getBuildArea(0), 20.0F, 16.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 105);
			t.addCondition(COND::MAX_FOOD, 2);
			t.addCondition(COND::MIN_FOOD_CAP, 31);
			more_pylons.addDirective(d);
			more_pylons.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 100);
			t2.addCondition(COND::MAX_FOOD, 6);
			t2.addCondition(COND::MIN_FOOD_CAP, 39);
			more_pylons.addTrigger(t2);
			bot->addStrat(more_pylons);
		}

		{	// build forge if the game continues
			Precept main_forge(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_FORGE, bot->locH->bases[0].getBuildArea(0), 20.0F, 16.0F);
			main_forge.addDirective(d);
			Trigger t(bot);
			t.addCondition(COND::MIN_TIME, 13000);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0));
			main_forge.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 700);
			t2.addCondition(COND::MIN_FOOD, 4);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0));
			main_forge.addTrigger(t);
			bot->addStrat(main_forge);
		}

		{	// build cannons if the game continues
			Precept main_cannon(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->locH->bases[0].getBuildArea(0), 20.0F, 16.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 5, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0));
			main_cannon.addDirective(d);
			main_cannon.addTrigger(t);
			bot->addStrat(main_cannon);
		}

		{	// start wrecking
			Precept attack_enemy_base(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation(), 4.0F);
			Trigger t(bot);
			d.excludeFlag(FLAGS::IS_FLYING);
			t.addCondition(COND::MAX_TIME, 4999);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_ZEALOT, bot->locH->getProxyLocation(), 20.0F);
			attack_enemy_base.addDirective(d);
			attack_enemy_base.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_UNIT_WITH_FLAGS, 5, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			t2.addCondition(COND::MAX_TIME, 4999);
			attack_enemy_base.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t3.addCondition(COND::MAX_TIME, 4999);
			attack_enemy_base.addTrigger(t3);
			bot->addStrat(attack_enemy_base);
		}

		{	// keep wrecking
			Precept attack_and_explore(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation(), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->smartAttackLocation(); };
			d.setTargetLocationFunction(this, bot, func);
			d.excludeFlag(FLAGS::IS_FLYING);
			d.setContinuous();
			d.setOverrideOther();
			t.addCondition(COND::MIN_TIME, 5000);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_ZEALOT, bot->locH->getProxyLocation(), 20.0F);
			attack_and_explore.addDirective(d);
			attack_and_explore.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_UNIT_WITH_FLAGS, 5, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			t2.addCondition(COND::MIN_TIME, 5000);
			attack_and_explore.addTrigger(t2);
			Trigger t3(bot);
			t3.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t3.addCondition(COND::MIN_TIME, 5000);
			attack_and_explore.addTrigger(t3);
			bot->addStrat(attack_and_explore);
		}
		{	// in case things don't end, start building up stuff to clean up
			Precept assim_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[0].getTownhall());
			assim_1.addDirective(d);
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_FOOD_USED, 37);
			d.setDebug(true);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
			assim_1.addTrigger(t);

			bot->addStrat(assim_1);
		}
		{	// in case things don't end, start building up stuff to clean up
			Precept cyber_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_CYBERNETICSCORE, bot->locH->bases[0].getBuildArea(0));
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 5, sc2::UNIT_TYPEID::PROTOSS_ZEALOT);
			t.addCondition(COND::MIN_FOOD_USED, 37);
			t.addCondition(COND::MIN_MINERALS, 200);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			cyber_1.addTrigger(t);
			cyber_1.addDirective(d);
			bot->addStrat(cyber_1);
		}
		{	// in case things don't end, start building up stuff to clean up
			Precept stargate_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_STARGATE, bot->locH->bases[0].getBuildArea(0));
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_STARGATE, bot->locH->bases[0].getTownhall(), 40.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			stargate_1.addTrigger(t);
			stargate_1.addDirective(d);
			bot->addStrat(stargate_1);
		}
		{	// train void rays for cleanup
			Precept train_voidray(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_STARGATE, sc2::ABILITY_ID::TRAIN_VOIDRAY);
			d.allowMultiple(); // more than one stargate can train at the same time
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 250);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_FOOD, 4);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_STARGATE);
			train_voidray.addDirective(d);
			train_voidray.addTrigger(t);
			bot->addStrat(train_voidray);
		}
		{	// void rays clean up
			Precept flyers_search(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_FLYING}, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation(), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->smartAttackFlyingLocation(); };
			d.setTargetLocationFunction(this, bot, func);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 4, std::unordered_set<FLAGS>{FLAGS::IS_FLYING});
			flyers_search.addDirective(d);
			flyers_search.addTrigger(t);
			Trigger t2(bot); // just in case somehow we have only one and can't build more, eventually just send it by itself
			t2.addCondition(COND::MIN_UNIT_WITH_FLAGS, 1, std::unordered_set<FLAGS>{FLAGS::IS_FLYING});
			t2.addCondition(COND::MIN_TIME, 14000);
			flyers_search.addTrigger(t2);
			bot->addStrat(flyers_search);
		}

		{	// designate 2 probes to not defend at all times, so our economy still runs
			Precept assign_non_defender(bot); 
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::ACTION_TYPE::SET_FLAG, sc2::UNIT_TYPEID::PROTOSS_PROBE, FLAGS::NON_DEFENDER, 20.0F);
			Trigger t(bot);
			t.addCondition(COND::MAX_UNIT_WITH_FLAGS, 2, std::unordered_set<FLAGS>{FLAGS::NON_DEFENDER});
			assign_non_defender.addTrigger(t);
			assign_non_defender.addDirective(d);
			bot->addStrat(assign_non_defender);
		}

		{	// send workers to defend base
			Precept workers_defend_base(bot);
			Directive d(Directive::MATCH_FLAGS_NEAR_LOCATION, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_WORKER}, sc2::ABILITY_ID::ATTACK, bot->locH->bases[0].getTownhall(), bot->locH->bases[1].getRallyPoint(), 8.0F, 3.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->smartStayHomeAndDefend(); };
			d.setTargetLocationFunction(this, bot, func);
			d.excludeFlag(FLAGS::NON_DEFENDER);
			d.excludeFlag(FLAGS::IS_PROXY);
			t.addCondition(COND::MIN_ENEMY_UNITS_NEAR_LOCATION, 1, bot->locH->bases[0].getTownhall());
			t.addCondition(COND::THREAT_EXISTS_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), 14.0F);
			workers_defend_base.addTrigger(t);
			workers_defend_base.addDirective(d);
			bot->addStrat(workers_defend_base);
		}

	}
}