#include <iostream>
#include <cassert>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "BasicSc2Bot.h"
#include "LadderInterface.h"

#include "Triggers.h"
#include "Strategy.h"

using namespace sc2;

int main(int argc, char* argv[]) {
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);
	const char* kProximaStationLE = "Ladder/ProximaStationLE.SC2Map"; // 3
	const char* kCactusValleyLE = "Ladder/CactusValleyLE.SC2Map";     // 1
	std::vector<const char*> map_strings;
	map_strings.push_back(kCactusValleyLE);
	map_strings.push_back(sc2::kMapBelShirVestigeLE);
	map_strings.push_back(kProximaStationLE);

	BotAgent bot;


	Strategy strategy(&bot);
	Human human;
	sc2::Agent* player_one;
	player_one = &human;
	bot.setCurrentStrategy(&strategy);
	int map_index;
	Race bot_race;
	Race opp_race;
	Difficulty difficulty;
	bool human_player;
	bool fullscreen;
	bool realtime;

	strategy.loadGameSettings(&map_index, &bot_race, &opp_race, &difficulty, &human_player, &fullscreen, &realtime);
	assert(map_index > 0 && map_index <= 3);
	if (human_player) {
		coordinator.SetParticipants({
			CreateParticipant(opp_race, &human),
			CreateParticipant(bot_race, &bot)
			});
	}
	else {
		coordinator.SetParticipants({
			CreateParticipant(bot_race, &bot),
			CreateComputer(opp_race, difficulty)
			});
	}
	coordinator.SetRealtime(realtime);
	coordinator.SetFullScreen(fullscreen);
	coordinator.LaunchStarcraft();
	coordinator.StartGame(map_strings[map_index - 1]);

	while (coordinator.Update()) {
	}
	return 0;
}