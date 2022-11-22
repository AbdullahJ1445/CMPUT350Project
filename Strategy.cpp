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
			bot->storeLocation("PYLON_1", sc2::Point2D(124.5,39));
			bot->storeLocation("PYLON_2", sc2::Point2D(124.5, 37));
			bot->storeLocation("CANNON_1", sc2::Point2D(125,41));
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
			Precept base_probe(bot);
			Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 50);
			t.add_condition(COND::MIN_FOOD, 1);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 24, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[0].get_townhall(), 40.0f);
			base_probe.addDirective(d);
			base_probe.addTrigger(t);
			bot->addStrat(base_probe);
		}
		{
			Precept first_expansion(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_NEXUS, bot->locH->bases[1].get_townhall());
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 400);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].get_townhall());
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].get_townhall(), 0.5F);
			first_expansion.addDirective(d);
			first_expansion.addTrigger(t);
			bot->addStrat(first_expansion);
		}
		{
			Precept pylon_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->getStoredLocation("PYLON_1"));
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 100);
			t.add_condition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 0.5F);
			pylon_1.addDirective(d);
			pylon_1.addTrigger(t);
			Trigger t2(bot);
			t2.add_condition(COND::MIN_MINERALS, 100);
			t2.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].get_townhall());
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_1"), 0.5F);
			pylon_1.addTrigger(t2);
			bot->addStrat(pylon_1);
		}
		{
			Precept forge_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_FORGE, bot->getStoredLocation("FORGE_1"));
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 150);
			t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			forge_1.addDirective(d);
			forge_1.addTrigger(t);
			Trigger t2(bot);
			t2.add_condition(COND::MIN_MINERALS, 150);
			t2.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].get_townhall());
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			forge_1.addTrigger(t2);
			bot->addStrat(forge_1);
		}
		{
			Precept gateway_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->getStoredLocation("GATEWAY_1"));
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 150);
			t.add_condition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			gateway_1.addDirective(d);
			gateway_1.addTrigger(t);
			Trigger t2(bot);
			t2.add_condition(COND::MIN_MINERALS, 150);
			t2.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			gateway_1.addTrigger(t2);
			bot->addStrat(gateway_1);
		}
		{
			Precept pylon_2(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->getStoredLocation("PYLON_2"));
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 100);
			t.add_condition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_2"), 0.5F);
			pylon_2.addDirective(d);
			pylon_2.addTrigger(t);
			Trigger t2(bot);
			t2.add_condition(COND::MIN_MINERALS, 100);
			t2.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, bot->getStoredLocation("GATEWAY_1"), 0.5F);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->getStoredLocation("PYLON_2"), 0.5F);
			pylon_2.addTrigger(t2);
			bot->addStrat(pylon_2);
		}
		{
			Precept cannon_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->getStoredLocation("CANNON_1"));
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 150);
			t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->getStoredLocation("CANNON_1"), 0.5F);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->getStoredLocation("CANNON_1"), 0.5F);
			cannon_1.addDirective(d);
			cannon_1.addTrigger(t);
			bot->addStrat(cannon_1);
		}
		{
			Precept cyber_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_CYBERNETICSCORE, bot->getStoredLocation("CYBER_1"));
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 150);			
			t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, bot->getStoredLocation("CYBER_1"), 0.5F);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, bot->getStoredLocation("CYBER_1"), 0.5F);
			cyber_1.addDirective(d);
			cyber_1.addTrigger(t);
			bot->addStrat(cyber_1);
		}
		{
			Precept assim_1(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[0].get_townhall());
			Trigger t(bot);
			t.add_condition(COND::MIN_MINERALS, 75);
			t.add_condition(COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[0].get_townhall());
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
			assim_1.addDirective(d);
			assim_1.addTrigger(t);
			Trigger t2(bot);
			t2.add_condition(COND::MIN_MINERALS, 75);
			t2.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[0].get_townhall());
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
			assim_1.addTrigger(t2);
			Trigger t3(bot);
			t3.add_condition(COND::MIN_MINERALS, 75);
			t3.add_condition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
			t3.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
			t3.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[0].get_townhall());
			t3.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
			assim_1.addTrigger(t3);
			bot->addStrat(assim_1);
		}
		{
			Precept defense_pylon(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[1].get_defend_point(0), 2.0F);
			Trigger t(bot);
			for (auto tc : startup_base_conds)
				t.add_condition(tc);
			t.add_condition(COND::MIN_MINERALS, 100);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[1].get_defend_point(0), 3.0F);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
			defense_pylon.addDirective(d);
			defense_pylon.addTrigger(t);
			bot->addStrat(defense_pylon);
		}
		{
			Precept more_cannons(bot);
			Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->locH->bases[1].get_defend_point(0), 6.0F);
			d.allowMultiple();
			Trigger t(bot);
			for (auto tc : startup_base_conds)
				t.add_condition(tc);
			t.add_condition(COND::MIN_MINERALS, 150);
			t.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[1].get_defend_point(0), 12.0F);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 4, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].get_defend_point(0), 6.0F);
			t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].get_defend_point(0), 6.0F);
			more_cannons.addDirective(d);
			more_cannons.addTrigger(t);
			Trigger t2(bot);  // allow up to 2 to be constructed at once here
			for (auto tc : startup_base_conds)
				t2.add_condition(tc);
			t2.add_condition(COND::MIN_MINERALS, 150);
			t2.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[1].get_defend_point(0), 12.0F);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].get_defend_point(0), 6.0F);
			t2.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].get_defend_point(0), 6.0F);
			more_cannons.addTrigger(t2);
			bot->addStrat(more_cannons);
		}
	}
	
}