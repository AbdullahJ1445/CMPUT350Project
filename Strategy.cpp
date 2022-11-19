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
	*bot_race = sc2::Race::Protoss;
	*opp_race = sc2::Race::Terran;
	*human_player = false;
	*fullscreen = false;
	*realtime = true;
}


void Strategy::loadStrategies() {

	// (1) Create a Precept - Contains the Full Order
	// (2) Create a Directive - What should happen when a trigger is met
	// (3) Create a Trigger - A trigger contains TriggerConditions, and is satisfied when all conditions are met
	// (4) Add TriggerConditions to the Trigger by using .add_condtion()
	// (5) repeat 3 and 4 if you wish for this directive to be executed under different conditions
	// (6) set the directive with .addDirective()
	// (7) add the trigger(s) with .addTrigger()
	// (8) add the Precept to the bot with bot->addStrat()
	
	
	bot->storeUnitType("_CHRONOBOOST_TARGET", sc2::UNIT_TYPEID::PROTOSS_GATEWAY);   // special tag to specify a unit that chronoboost will only target
	

	{
		Precept base_probe(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 50);
		t.add_condition(COND::MIN_FOOD, 1);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 15, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[0].get_townhall(), 18.0f);
		base_probe.addDirective(d);
		base_probe.addTrigger(t);
		bot->addStrat(base_probe);
	}
	{
		Precept choke_pylon(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].get_defend_point(0), 7.0f);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 100);
		t.add_condition(COND::MAX_FOOD, 4);
		t.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_build_area(0));
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_defend_point(0), 8.0f);
		choke_pylon.addDirective(d);
		choke_pylon.addTrigger(t);
		bot->addStrat(choke_pylon);
	}
	{
		Precept use_chrono(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, bot->locH->bases[0].get_townhall());
		Trigger t(bot);
		t.add_condition(COND::HAS_ABILITY_READY, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST);
		use_chrono.addDirective(d);
		use_chrono.addTrigger(t);
		bot->addStrat(use_chrono);
	}
	{
		Precept base_pylon(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].get_build_area(0));
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 100);
		t.add_condition(COND::MAX_FOOD, 4);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_build_area(0), 13.0F);
		base_pylon.addDirective(d);
		base_pylon.addTrigger(t);
		Trigger t2(bot);
		t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_build_area(0), 13.0F);
		t2.add_condition(COND::MIN_FOOD_CAP, 20);
		base_pylon.addTrigger(t2);
		bot->addStrat(base_pylon);
	}
	{
		Precept pylon_by_townhall(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].get_townhall(), 8.0F);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 100);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_townhall(), 9.0F);
		pylon_by_townhall.addDirective(d);
		pylon_by_townhall.addTrigger(t);
		bot->addStrat(pylon_by_townhall);
	}
	{
		Precept base_pylon_2(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[0].get_build_area(1));
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 100);
		t.add_condition(COND::MAX_FOOD, 4);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_build_area(0));
		base_pylon_2.addDirective(d);
		base_pylon_2.addTrigger(t);
		Trigger t2(bot);
		t2.add_condition(COND::MIN_MINERALS, 100);
		t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_build_area(1));
		t2.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t2.add_condition(COND::MIN_FOOD_CAP, 24);
		base_pylon_2.addTrigger(t2);
		Trigger t3(bot);
		t3.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_build_area(1));
		t3.add_condition(COND::MIN_MINERALS, 405);
		t3.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_PYLON);
		t.add_condition(COND::MAX_FOOD, 4);
		base_pylon_2.addTrigger(t3);
		bot->addStrat(base_pylon_2);
	}
	{
		Precept build_forge(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_FORGE, bot->locH->bases[0].get_build_area(0));
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		build_forge.addDirective(d);
		build_forge.addTrigger(t);
		bot->addStrat(build_forge);
	}
	{
		Precept base_cannon(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->locH->bases[0].get_build_area(0));
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 3, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[0].get_build_area(0));
		base_cannon.addDirective(d);
		base_cannon.addTrigger(t);
		bot->addStrat(base_cannon);
	}
	{
		Precept base_cannon_2(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->locH->bases[0].get_build_area(1));
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		t.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[0].get_build_area(1));
		t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[0].get_build_area(1));
		base_cannon_2.addDirective(d);
		base_cannon_2.addTrigger(t);
		Trigger t2(bot);
		t2.add_condition(COND::MIN_MINERALS, 150);
		t2.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 2, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[0].get_build_area(1));
		t2.add_condition(COND::BASE_IS_ACTIVE, 1);
		base_cannon_2.addTrigger(t2);
		bot->addStrat(base_cannon_2);
	}
	{
		Precept choke_cannon(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->locH->bases[0].get_defend_point(0), 7.0f);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 2, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[0].get_defend_point(0), 7.0f);
		choke_cannon.addDirective(d);
		choke_cannon.addTrigger(t);
		bot->addStrat(choke_cannon);
	}
	{
		Precept base_assimilator(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[0].get_townhall());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 75);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_FORGE);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[0].get_townhall());
		base_assimilator.addDirective(d);
		base_assimilator.addTrigger(t);
		Trigger t2(bot);
		t2.add_condition(COND::MIN_MINERALS, 75);
		t2.add_condition(COND::BASE_IS_ACTIVE, 1);
		t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[0].get_townhall());
		base_assimilator.addTrigger(t2);
		bot->addStrat(base_assimilator);
	}
	{
		Precept build_cyber_core(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_CYBERNETICSCORE, bot->locH->bases[0].get_random_build_area());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 200);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
		build_cyber_core.addDirective(d);
		build_cyber_core.addTrigger(t);
		bot->addStrat(build_cyber_core);
	}
	{
		Precept base_gateway(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_GATEWAY, bot->locH->bases[0].get_random_build_area());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		base_gateway.addDirective(d);
		base_gateway.addTrigger(t);
		Trigger t2(bot);
		t2.add_condition(COND::MIN_MINERALS, 150);
		t2.add_condition(COND::MAX_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t2.add_condition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::BLINKTECH);
		base_gateway.addTrigger(t2);
		Trigger t3(bot);
		t3.add_condition(COND::MIN_MINERALS, 150);
		t3.add_condition(COND::MAX_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_GATEWAY);
		t3.add_condition(COND::BASE_IS_ACTIVE, 1);
		base_gateway.addTrigger(t3);
		bot->addStrat(base_gateway);
	}
	{
		Precept base_twi_council(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_TWILIGHTCOUNCIL, bot->locH->bases[0].get_random_build_area());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_GAS, 100);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
		t.add_condition(COND::BASE_IS_ACTIVE, 1);
		base_twi_council.addDirective(d);
		base_twi_council.addTrigger(t);
		bot->addStrat(base_twi_council);
	}
	{
		Precept train_stalker(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_GATEWAY, sc2::ABILITY_ID::TRAIN_STALKER);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 125);
		t.add_condition(COND::MIN_GAS, 50);
		t.add_condition(COND::MIN_FOOD, 2);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 15, sc2::UNIT_TYPEID::PROTOSS_STALKER);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
		train_stalker.addDirective(d);
		train_stalker.addTrigger(t);
		bot->addStrat(train_stalker);
	}
	{
		Precept research_blink(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, sc2::ABILITY_ID::RESEARCH_BLINK);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_GAS, 150);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL);
		t.add_condition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::BLINKTECH, false);
		research_blink.addDirective(d);
		research_blink.addTrigger(t);
		bot->addStrat(research_blink);
	}
	{
		Precept upgrade_attack(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_FORGE, sc2::ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 100);
		t.add_condition(COND::MIN_GAS, 100);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
		upgrade_attack.addDirective(d);
		upgrade_attack.addTrigger(t);
		bot->addStrat(upgrade_attack);
	}
	{
		Precept send_small_scout(bot);
		std::unordered_set<FLAGS> attackers;
		attackers.insert(FLAGS::IS_ATTACKER);
		Directive d(Directive::MATCH_FLAGS, Directive::NEAR_LOCATION, attackers, sc2::ABILITY_ID::ATTACK, bot->locH->bases[2].get_townhall());
		Trigger t(bot);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_STALKER);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 5, sc2::UNIT_TYPEID::PROTOSS_STALKER);
		send_small_scout.addDirective(d);
		send_small_scout.addTrigger(t);
		bot->addStrat(send_small_scout);
	}
	{
		int num_bases = (bot->locH->bases).size();
		std::unordered_set<FLAGS> attackers;
		for (int i = 2; i < num_bases - 1; i++) {
			Precept continue_scout(bot);
			Directive d(Directive::MATCH_FLAGS_NEAR_LOCATION, Directive::NEAR_LOCATION, attackers, sc2::ABILITY_ID::ATTACK, bot->locH->bases[i].get_townhall(), bot->locH->bases[i + 1].get_townhall(), 22.0F);
			Trigger t(bot);
			t.add_condition(COND::MIN_UNIT_WITH_FLAGS_NEAR_LOCATION, i + 1, attackers, bot->locH->bases[i].get_townhall());
			continue_scout.addDirective(d);
			continue_scout.addTrigger(t);
			bot->addStrat(continue_scout);
		}
	}
	{
		// When we have 2 Collosus, send all of our "attackers" to the enemy base
		Precept send_entire_army(bot);
		std::unordered_set<FLAGS> attackers;
		attackers.insert(FLAGS::IS_ATTACKER);
		Directive d(Directive::MATCH_FLAGS, Directive::NEAR_LOCATION, attackers, sc2::ABILITY_ID::ATTACK, bot->enemy_location);
		Trigger t(bot);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
		send_entire_army.addDirective(d);
		send_entire_army.addTrigger(t);
		bot->addStrat(send_entire_army);
	}
	{
		// If there are 5 stalkers in enemy base, send an attack to their first expansion
		Precept send_entire_army_2(bot);
		std::unordered_set<FLAGS> attackers;
		attackers.insert(FLAGS::IS_ATTACKER);
		Directive d(Directive::MATCH_FLAGS, Directive::NEAR_LOCATION, attackers, sc2::ABILITY_ID::ATTACK, bot->locH->bases.back().get_townhall());
		Trigger t(bot);
		t.add_condition(COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION, 5, sc2::UNIT_TYPEID::PROTOSS_STALKER, bot->enemy_location);
		send_entire_army_2.addDirective(d);
		send_entire_army_2.addTrigger(t);
		bot->addStrat(send_entire_army_2);
	}
	{
		Precept first_expansion(bot);
		Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_NEXUS, bot->locH->bases[1].get_townhall());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 400);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_NEXUS, bot->locH->bases[1].get_townhall());
		first_expansion.addDirective(d);
		first_expansion.addTrigger(t);
		bot->addStrat(first_expansion);
	}
	{
		Precept pylon_exp1(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PYLON, bot->locH->bases[1].get_build_area(0));
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 100);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PYLON, bot->locH->bases[1].get_build_area(0));
		t.add_condition(COND::BASE_IS_ACTIVE, 1);
		pylon_exp1.addDirective(d);
		pylon_exp1.addTrigger(t);
		bot->addStrat(pylon_exp1);
	}
	{
		Precept exp1_probe(bot);
		Directive d(Directive::UNIT_TYPE, Directive::EXACT_LOCATION, sc2::UNIT_TYPEID::PROTOSS_NEXUS, sc2::ABILITY_ID::TRAIN_PROBE, bot->locH->bases[1].get_townhall());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 50);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 15, sc2::UNIT_TYPEID::PROTOSS_PROBE, bot->locH->bases[1].get_townhall());
		t.add_condition(COND::BASE_IS_ACTIVE, 1);
		exp1_probe.addDirective(d);
		exp1_probe.addTrigger(t);
		bot->addStrat(exp1_probe);
	}
	{
		Precept exp1_assimilator(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ASSIMILATOR, bot->locH->bases[1].get_townhall());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 75);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 0, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[1].get_townhall());
		t.add_condition(COND::BASE_IS_ACTIVE, 1);
		exp1_assimilator.addDirective(d);
		exp1_assimilator.addTrigger(t);
		Trigger t2(bot);
		t2.add_condition(COND::MIN_MINERALS, 75);
		t2.add_condition(COND::MAX_GAS, 200);
		t2.add_condition(COND::BASE_IS_ACTIVE, 1);
		t2.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, bot->locH->bases[1].get_townhall());
		exp1_assimilator.addTrigger(t2);
		bot->addStrat(exp1_assimilator);
	}
	{
		Precept cannon_exp1(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_PHOTONCANNON, bot->locH->bases[1].get_build_area(0));
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION, 1, sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, bot->locH->bases[1].get_build_area(0));
		t.add_condition(COND::BASE_IS_ACTIVE, 1);
		cannon_exp1.addDirective(d);
		cannon_exp1.addTrigger(t);
		bot->addStrat(cannon_exp1);
	}
	{
		Precept robotics_fac_exp1(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ROBOTICSFACILITY, bot->locH->bases[1].get_build_area(0));
		Trigger t(bot);
		t.add_condition(COND::BASE_IS_ACTIVE, 1);
		t.add_condition(COND::MIN_FOOD_USED, 24);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_GAS, 100);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
		robotics_fac_exp1.addDirective(d);
		robotics_fac_exp1.addTrigger(t);
		bot->addStrat(robotics_fac_exp1);
	}
	{
		Precept train_immortal(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, sc2::ABILITY_ID::TRAIN_IMMORTAL);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 275);
		t.add_condition(COND::MIN_GAS, 100);
		t.add_condition(COND::MIN_FOOD, 4);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 3, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
		train_immortal.addDirective(d);
		train_immortal.addTrigger(t);
		bot->addStrat(train_immortal);
	}
	{
		Precept build_robotics_bay_main(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ROBOTICSBAY, bot->locH->bases[0].get_random_build_area());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_GAS, 150);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
		build_robotics_bay_main.addDirective(d);
		build_robotics_bay_main.addTrigger(t);
		bot->addStrat(build_robotics_bay_main);
	}
	{
		Precept build_robotics_bay_exp1(bot);
		Directive d(Directive::UNIT_TYPE, Directive::NEAR_LOCATION, sc2::UNIT_TYPEID::PROTOSS_PROBE, sc2::ABILITY_ID::BUILD_ROBOTICSBAY, bot->locH->bases[1].get_random_build_area());
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_GAS, 150);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 4, sc2::UNIT_TYPEID::PROTOSS_IMMORTAL);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 0, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
		build_robotics_bay_exp1.addDirective(d);
		build_robotics_bay_exp1.addTrigger(t);
		bot->addStrat(build_robotics_bay_exp1);
	}
	{
		Precept train_colossus(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, sc2::ABILITY_ID::TRAIN_COLOSSUS);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 300);
		t.add_condition(COND::MIN_GAS, 200);
		t.add_condition(COND::MIN_FOOD, 6);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
		t.add_condition(COND::MAX_UNIT_OF_TYPE, 2, sc2::UNIT_TYPEID::PROTOSS_COLOSSUS);
		train_colossus.addDirective(d);
		train_colossus.addTrigger(t);
		bot->addStrat(train_colossus);
	}
	{
		Precept research_thermal_lance(bot);
		Directive d(Directive::UNIT_TYPE, Directive::SIMPLE_ACTION, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY, sc2::ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE);
		Trigger t(bot);
		t.add_condition(COND::MIN_MINERALS, 150);
		t.add_condition(COND::MIN_GAS, 150);
		t.add_condition(COND::MIN_UNIT_OF_TYPE, 1, sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY);
		t.add_condition(COND::HAVE_UPGRADE, sc2::UPGRADE_ID::EXTENDEDTHERMALLANCE, false);
		research_thermal_lance.addDirective(d);
		research_thermal_lance.addTrigger(t);
		bot->addStrat(research_thermal_lance);
	}
}