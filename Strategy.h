#pragma once
#include "Agents.h"
#include "sc2api/sc2_api.h"

struct Strategy {
	Strategy(BotAgent* bot_);
	void loadGameSettings(int *map_index, sc2::Race *bot_race, sc2::Race *opp_race, sc2::Difficulty *difficulty, bool *human_player=false, bool *fullscreen=false, bool *realtime=false);
	void loadStrategies();
	BotAgent* bot;
private:
	std::vector<const char*> maps;
};