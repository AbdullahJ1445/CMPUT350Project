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
}


void Strategy::loadStrategies() {


	

	int map_index = bot->getMapIndex();
	int p_id = bot->locH->getPlayerIDForMap(map_index, bot->Observation()->GetStartLocation());

	// Cactus Valley Strategy
	if (map_index == 1) {

		// initialize cactus valley first structure locations

		if (p_id == 1) {
			bot->storeLocation("FORGE_1", sc2::Point2D(68.5, 148.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(68.0, 153.0));
			bot->storeLocation("PYLON_2", sc2::Point2D(68, 155));
			bot->storeLocation("CANNON_1", sc2::Point2D(67.0, 151.0));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(71.5, 150.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(67.5, 157.5));
		}
		if (p_id == 2) {
			bot->storeLocation("FORGE_1", sc2::Point2D(148.5,123.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(153,124));
			bot->storeLocation("PYLON_2", sc2::Point2D(155, 124));
			bot->storeLocation("CANNON_1", sc2::Point2D(151,125));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(150.5,120.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(157.5,124.5));
		}
		if (p_id == 3) {
			bot->storeLocation("FORGE_1", sc2::Point2D(123.5, 43.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(124, 39));
			bot->storeLocation("PYLON_2", sc2::Point2D(124, 37));
			bot->storeLocation("CANNON_1", sc2::Point2D(125, 41));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(120.5,41.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(124.5,34.5));
		}
		if (p_id == 4) {
			bot->storeLocation("FORGE_1", sc2::Point2D(43.5,68.5));
			bot->storeLocation("PYLON_1", sc2::Point2D(39,68));
			bot->storeLocation("PYLON_2", sc2::Point2D(37, 68));
			bot->storeLocation("CANNON_1", sc2::Point2D(41,67));
			bot->storeLocation("GATEWAY_1", sc2::Point2D(41.5,71.5));
			bot->storeLocation("CYBER_1", sc2::Point2D(34.5,67.5));
		}
		
		// pre-create a container of TriggerConditions to reduce copy/pasting same conditions over and over
		// all of these being met means the initial buildings are intact, so we won't create a building in their place

		std::vector<Trigger::TriggerCondition> startup_base_conds;
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 0.5F));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_2"), 0.5F));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE, bot->getStoredLocation("FORGE_1"), 0.5F));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->getStoredLocation("CANNON_1"), 0.5F));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F));
		startup_base_conds.push_back(Trigger::TriggerCondition(bot, COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, bot->getStoredLocation("CYBER_1"), 0.5F));


		{
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
		{
			Precept main_pylon_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].getBuildArea(0), 12.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MAX_FOOD, 4);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 8, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			main_pylon_2.addDirective(d);
			main_pylon_2.addTrigger(t);
			bot->addStrat(main_pylon_2);
		}
		{
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
		{
			Precept base_probe(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 50);
			t.addCondition(COND::MIN_FOOD, 1);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 18, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[0].getTownhall(), 12.0f);
			base_probe.addDirective(d);
			base_probe.addTrigger(t);
			bot->addStrat(base_probe);
		}
		{
			Precept exp_probe(bot);
			Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[1].getTownhall(), Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 50);
			t.addCondition(COND::MIN_FOOD, 1);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 18, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[1].getTownhall(), 12.0f);
			exp_probe.addDirective(d);
			exp_probe.addTrigger(t);
			bot->addStrat(exp_probe);
		}
		{
			Precept first_expansion(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_NEXUS, bot->locH->bases[1].getTownhall());
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 400);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall());
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall(), 0.5F);
			first_expansion.addDirective(d);
			first_expansion.addTrigger(t);
			bot->addStrat(first_expansion);
		}
		{
			Precept pylon_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->getStoredLocation("PYLON_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 0.5F);
			pylon_1.addDirective(d);
			pylon_1.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 100);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall());
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 0.5F);
			pylon_1.addTrigger(t2);
			bot->addStrat(pylon_1);
		}
		{
			Precept forge_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_FORGE, bot->getStoredLocation("FORGE_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			forge_1.addDirective(d);
			forge_1.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 150);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].getTownhall());
			t2.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			forge_1.addTrigger(t2);
			bot->addStrat(forge_1);
		}
		{
			Precept gateway_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->getStoredLocation("GATEWAY_1"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			gateway_1.addDirective(d);
			gateway_1.addTrigger(t);
			bot->addStrat(gateway_1);
		}
		{
			Precept gateway_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->locH->bases[0].getBuildArea(1), 10.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 700);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 7, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			gateway_2.addDirective(d);
			gateway_2.addTrigger(t);
			bot->addStrat(gateway_2);
		}
		{
			Precept pylon_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->getStoredLocation("PYLON_2"));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_2"), 0.3F);
			pylon_2.addDirective(d);
			pylon_2.addTrigger(t);
			bot->addStrat(pylon_2);
		}
		{
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
		{
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
		{
			Precept assim_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[0].getTownhall());
			Trigger t(bot);
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
		{
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
		{
			Precept more_cannons(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->locH->bases[1].getDefendPoint(0), 6.0F);
			d.allowMultiple();
			Trigger t(bot);
			for (auto tc : startup_base_conds)
				t.addCondition(tc);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 2, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[1].getDefendPoint(0), 12.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 4, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].getDefendPoint(0), 6.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].getDefendPoint(0), 6.0F);
			more_cannons.addDirective(d);
			more_cannons.addTrigger(t);
			bot->addStrat(more_cannons);
		}
		{
			Precept train_stalker(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_STALKER);
			d.allowMultiple();
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 125);
			t.addCondition(COND::MIN_GAS, 50);
			t.addCondition(COND::MIN_FOOD, 2);
			t.addCondition(COND::MAX_UNIT_OF_TYPE, 15, sc2::UNIT_TYPEID::PROTOSS_STALKER);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			train_stalker.addDirective(d);
			train_stalker.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_MINERALS, 600);
			t2.addCondition(COND::MIN_GAS, 200);
			t2.addCondition(COND::MIN_FOOD, 2);
			t2.addCondition(COND::MAX_UNIT_OF_TYPE, 31, sc2::UNIT_TYPEID::PROTOSS_STALKER);
			t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			train_stalker.addTrigger(t2);
			bot->addStrat(train_stalker);
		}
		{
			Precept main_gateway(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->locH->bases[0].getBuildArea(0));
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 100);
			t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0), 8.0F);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			main_gateway.addDirective(d);
			main_gateway.addTrigger(t);
			bot->addStrat(main_gateway);
		}
		{
			Precept twilight_council(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_TWILIGHTCOUNCIL, bot->locH->bases[0].getBuildArea(1), 12.0F);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			//t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0), 8.0F);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			//t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			twilight_council.addDirective(d);
			twilight_council.addTrigger(t);
			bot->addStrat(twilight_council);
		}
		{
			Precept twilight_council_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_TWILIGHTCOUNCIL, bot->locH->bases[0].getBuildArea(0), 14.0F);
			Trigger t(bot);
			d.allowMultiple();
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			//t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].getBuildArea(0), 8.0F);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			//t.addCondition(COND::MIN_UNIT_OF_TYPE_TOTAL, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			twilight_council_2.addDirective(d);
			twilight_council_2.addTrigger(t);
			bot->addStrat(twilight_council_2);
		}
		{
			Precept research_blink(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, sc2::ABILITY_ID::RESEARCH_BLINK);
			Trigger t(bot);
			t.addCondition(COND::MIN_MINERALS, 150);
			t.addCondition(COND::MIN_GAS, 150);
			t.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
			t.addCondition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::BLINKTECH, false);
			research_blink.addDirective(d);
			research_blink.addTrigger(t);
			bot->addStrat(research_blink);
		}
		{
			Precept assim_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[1].getTownhall());
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
		{
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
		{
			Precept use_chrono(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, bot->locH->bases[0].getTownhall());
			Trigger t(bot);
			t.addCondition(COND::MIN_FOOD_CAP, 16);
			t.addCondition(COND::HAS_ABILITY_READY, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST);
			use_chrono.addDirective(d);
			use_chrono.addTrigger(t);
			bot->addStrat(use_chrono);
		}
		{
			Precept attack_threats(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->getStoredLocation("CANNON_1"), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->getHighestThreatLocation(); };
			d.setTargetLocationFunction(this, bot, func);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 7, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			t.addCondition(COND::MAX_UNIT_WITH_FLAGS, 12, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			t.addCondition(COND::MAX_FOOD_CAP, 65);			
			attack_threats.addDirective(d);
			attack_threats.addTrigger(t);
			Trigger t2(bot);
			t2.addCondition(COND::MIN_UNIT_WITH_FLAGS_NEAR_LOCATION, 5, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, bot->locH->bases[1].getDefendPoint(0), 18.0F);
			t2.addCondition(COND::MAX_UNIT_WITH_FLAGS, 11, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			attack_threats.addTrigger(t2);
			bot->addStrat(attack_threats);
		} 
		{
			Precept attack_and_explore(bot);
			Directive d(Directive::MATCH_FLAGS, Directive::ACTION_TYPE::NEAR_LOCATION, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER}, sc2::ABILITY_ID::ATTACK, bot->getStoredLocation("CANNON_1"), 4.0F);
			Trigger t(bot);
			auto func = [this]() { return bot->locH->smartAttackLocation(); };
			d.setTargetLocationFunction(this, bot, func);
			t.addCondition(COND::MIN_UNIT_WITH_FLAGS, 12, std::unordered_set<FLAGS>{FLAGS::IS_ATTACKER});
			attack_and_explore.addDirective(d);
			attack_and_explore.addTrigger(t);
			bot->addStrat(attack_and_explore);
		}
	}
	else {

	bot->storeUnitType("_CHRONOBOOST_TARGET", sc2::UNIT_TYPEID::PROTOSS_GATEWAY);   // special tag to specify a unit that chronoboost will only target

	{
		Precept send_proxy(bot);
		Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::GENERAL_MOVE, bot->locH->getProxyLocation());
		Trigger t(bot);
		t.addCondition(COND::MAX_TIME, 1);
		send_proxy.addDirective(d);
		send_proxy.addTrigger(t);
		bot->addStrat(send_proxy);
	}
	{   // Prevent the proxy probe from trying to harvest minerals when idle
		Precept disable_default(bot);
		Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->getProxyLocation(), Directive::DISABLE_DEFAULT_DIRECTIVE, sc2::UNIT_TYPEID::PROTOSS_PROBE, 20.0F);
		Trigger t(bot);
		t.addCondition(COND::MAX_TIME, 2500);
		disable_default.addDirective(d);
		disable_default.addTrigger(t);
		bot->addStrat(disable_default);
	}
	{
		Precept base_probe(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
		Trigger t(bot);
		t.addCondition(COND::MIN_MINERALS, 50);
		t.addCondition(COND::MIN_FOOD, 1);
		t.addCondition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 13, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[0].getTownhall(), 18.0f);
		base_probe.addDirective(d);
		base_probe.addTrigger(t);
		bot->addStrat(base_probe);
	}
	{
		Precept proxy_pylon(bot);
		Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->getProxyLocation(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->getProxyLocation(), 30.0F);
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
	{
		Precept proxy_gateway(bot);
		Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->getProxyLocation(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->locH->getProxyLocation());
		Trigger t(bot);
		t.addCondition(COND::MIN_MINERALS, 150);
		t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->getProxyLocation(), 30.0F);
		t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->getProxyLocation());
		t.addCondition(COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->locH->getProxyLocation());
		proxy_gateway.addDirective(d);
		proxy_gateway.addTrigger(t);
		bot->addStrat(proxy_gateway);
	}
	{
		Precept use_chrono(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, bot->locH->bases[0].getTownhall());
		d.allowMultiple();
		Trigger t(bot);
		t.addCondition(COND::HAS_ABILITY_READY, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST);
		use_chrono.addDirective(d);
		use_chrono.addTrigger(t);
		bot->addStrat(use_chrono);
	}
	{
		Precept train_zealot(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_ZEALOT);
		d.allowMultiple(); // more than one gateway can train at the same time
		Trigger t(bot);
		t.addCondition(COND::MIN_MINERALS, 100);
		t.addCondition(COND::MIN_FOOD, 2);
		t.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		train_zealot.addDirective(d);
		train_zealot.addTrigger(t);
		Trigger t2(bot);
		t2.addCondition(COND::MIN_MINERALS, 100);
		t2.addCondition(COND::MIN_FOOD, 2);
		t2.addCondition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t2.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t2.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t2.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		train_zealot.addTrigger(t2);
		Trigger t3(bot);
		t3.addCondition(COND::MIN_MINERALS, 100);
		t3.addCondition(COND::MIN_FOOD, 2);
		t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t3.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t3.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t3.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t3.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		train_zealot.addTrigger(t3);
		Trigger t4(bot);
		t4.addCondition(COND::MIN_MINERALS, 100);
		t4.addCondition(COND::MIN_FOOD, 2);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		train_zealot.addTrigger(t4);
		Trigger t5(bot);
		t4.addCondition(COND::MIN_MINERALS, 100);
		t4.addCondition(COND::MIN_FOOD, 2);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t4.addCondition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		train_zealot.addTrigger(t5);
		bot->addStrat(train_zealot);
	}
	{
		Precept send_attack(bot);
		std::unordered_set<FLAGS> attackers;
		attackers.insert(FLAGS::IS_ATTACKER);
		Directive d(Directive::MATCH_FLAGS, Directive::NEAR_LOCATION, attackers, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation());
		Trigger t(bot);
		t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_ZEALOT, bot->locH->getProxyLocation());
		send_attack.addDirective(d);
		send_attack.addTrigger(t);
		bot->addStrat(send_attack);
	}
	{
		Precept send_probe_attack(bot);
		Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->getProxyLocation(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation(), 20.0F);
		Trigger t(bot);
		t.addCondition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_ZEALOT);
		send_probe_attack.addDirective(d);
		send_probe_attack.addTrigger(t);
		bot->addStrat(send_probe_attack);
	}
	{
		Precept more_pylons(bot);
		Directive d(Directive::UNIT_TYPE_NEAR_LOCATION, bot->locH->bases[0].getTownhall(), Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].getBuildArea(0), 20.0F, 16.0F);
		Trigger t(bot);
		t.addCondition(COND::MIN_MINERALS, 105);
		t.addCondition(COND::MAX_FOOD, 2);
		t.addCondition(COND::MIN_FOOD_CAP, 31);
		more_pylons.addDirective(d);
		more_pylons.addTrigger(t);
		bot->addStrat(more_pylons);
	}
	{
		Precept send_attack_exp(bot);
		std::unordered_set<FLAGS> attackers;
		attackers.insert(FLAGS::IS_ATTACKER);
		Directive d(Directive::MATCH_FLAGS, Directive::NEAR_LOCATION, attackers, sc2::ABILITY_ID::ATTACK, bot->locH->getBestEnemyLocation(), 40.0F);
		Trigger t(bot);
		t.addCondition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 7, sc2::UNIT_TYPEID::PROTOSS_ZEALOT, bot->locH->getBestEnemyLocation(), 40.0F);
		send_attack_exp.addDirective(d);
		send_attack_exp.addTrigger(t);
		bot->addStrat(send_attack_exp);
	}
	}
	
}