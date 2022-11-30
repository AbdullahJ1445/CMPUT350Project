#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "BasicSc2Bot.h"
#include "LadderInterface.h"
#include "BasicSc2Bot.h"

// LadderInterface allows the bot to be tested against the built-in AI or
// played against other bots
int main(int argc, char* argv[]) {
	BasicSc2Bot* mybot = new BasicSc2Bot();
	runBot(argc, argv, mybot, sc2::Race::Protoss);
	return 0;
}
