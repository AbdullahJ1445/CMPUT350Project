#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "BasicSc2Bot.h"
#include "LadderInterface.h"

#include "Agents.h"
#include "Triggers.h"

using namespace sc2;

int main(int argc, char* argv[]) {
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);
	const char* kProximaStationLE = "Ladder/ProximaStationLE.SC2Map";
	const char* kCactusValleyLE = "Ladder/CactusValleyLE.SC2Map";
	bool human_player = false;

	BotAgent bot;
	Human human;
	sc2::Agent* player_one;
	player_one = &human;

	if (human_player) {
		coordinator.SetParticipants({
			CreateParticipant(Race::Zerg, &human),
			CreateParticipant(Race::Protoss, &bot)
			});
	}
	else {
		coordinator.SetParticipants({
			CreateParticipant(Race::Protoss, &bot),
			CreateComputer(Race::Zerg)
			});
	}

	//coordinator.SetRealtime(true);
	//coordinator.SaveReplayList("C:/");
	//coordinator.SetFullScreen(true);
	coordinator.LaunchStarcraft();
	//coordinator.StartGame(kCactusValleyLE);
	//coordinator.StartGame(kMapBelShirVestigeLE);
	coordinator.StartGame(kProximaStationLE);

	while (coordinator.Update()) {
	}

	return 0;
}