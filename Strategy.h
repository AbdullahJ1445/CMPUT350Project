#pragma once
#include "Agents.h"
#include "sc2api/sc2_api.h"

struct Strategy {
	Strategy(BotAgent* bot_);
	void loadGameSettings(int *map_index, sc2::Race *bot_race, sc2::Race *opp_race, bool *human_player=false, bool *fullscreen=false, bool *realtime=false);
	void loadStrategies();
private:
	BotAgent* bot;
	std::vector<const char*> maps;
};