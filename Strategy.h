#pragma once
#include "BasicSc2Bot.h"
#include "sc2api/sc2_api.h"

struct Strategy {
	Strategy(BasicSc2Bot* bot_);
	void loadGameSettings(int *map_index, sc2::Race *bot_race, sc2::Race *opp_race, sc2::Difficulty *difficulty, bool *human_player=false, bool *fullscreen=false, bool *realtime=false);
	void loadStrategies_01();
	void loadStrategies_02();
	void loadStrategies_03();
	void loadStrategies_04();
	void loadStrategies_05();
	void loadStrategies_06();
	void loadStrategies_07();
	void loadStrategies_08();
	BasicSc2Bot* bot;
private:
	std::vector<const char*> maps;
	Strategy* strategy_ptr;
	int map_index;
	int p_id;
};