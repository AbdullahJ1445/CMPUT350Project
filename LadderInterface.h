
std::string kDefaultMap = "BelshirVestigeLE.SC2Map";

static sc2::Difficulty getDifficultyFromString(const std::string &InDifficulty)
{
	if (InDifficulty == "VeryEasy")
	{
		return sc2::Difficulty::VeryEasy;
	}
	if (InDifficulty == "Easy")
	{
		return sc2::Difficulty::Easy;
	}
	if (InDifficulty == "Medium")
	{
		return sc2::Difficulty::Medium;
	}
	if (InDifficulty == "MediumHard")
	{
		return sc2::Difficulty::MediumHard;
	}
	if (InDifficulty == "Hard")
	{
		return sc2::Difficulty::Hard;
	}
if (InDifficulty == "HardVeryHard")
{
	return sc2::Difficulty::HardVeryHard;
}
if (InDifficulty == "VeryHard")
{
	return sc2::Difficulty::VeryHard;
}
if (InDifficulty == "CheatVision")
{
	return sc2::Difficulty::CheatVision;
}
if (InDifficulty == "CheatMoney")
{
	return sc2::Difficulty::CheatMoney;
}
if (InDifficulty == "CheatInsane")
{
	return sc2::Difficulty::CheatInsane;
}

return sc2::Difficulty::Easy;
}

static sc2::Race getRaceFromString(const std::string & RaceIn)
{
	std::string race(RaceIn);
	std::transform(race.begin(), race.end(), race.begin(), ::tolower);

	if (race == "terran")
	{
		return sc2::Race::Terran;
	}
	else if (race == "protoss")
	{
		return sc2::Race::Protoss;
	}
	else if (race == "zerg")
	{
		return sc2::Race::Zerg;
	}
	else if (race == "random")
	{
		return sc2::Race::Random;
	}

	return sc2::Race::Random;
}

struct ConnectionOptions
{
	int32_t gamePort;
	int32_t startPort;
	std::string serverAddress;
	bool computerOpponent;
	sc2::Difficulty computerDifficulty;
	sc2::Race computerRace;
	std::string opponentId;
	std::string map;
};

static void parseArguments(int argc, char *argv[], ConnectionOptions &connect_options)
{
	sc2::ArgParser arg_parser(argv[0]);
	arg_parser.AddOptions({
		{ "-g", "--GamePort", "Port of client to connect to", false },
		{ "-o", "--StartPort", "Starting server port", false },
		{ "-l", "--LadderServer", "Ladder server address", false },
		{ "-c", "--ComputerOpponent", "If we set up a computer oppenent" },
		{ "-a", "--ComputerRace", "Race of computer oppent"},
		{ "-d", "--ComputerDifficulty", "Difficulty of computer oppenent"},
		{ "-m", "--Map", "Map to play on against computer opponent", },
		{ "-x", "--OpponentId", "PlayerId of opponent"}
		});
	arg_parser.Parse(argc, argv);
	std::string GamePortStr;
	if (arg_parser.Get("GamePort", GamePortStr)) {
		connect_options.gamePort = atoi(GamePortStr.c_str());
	}
	std::string StartPortStr;
	if (arg_parser.Get("StartPort", StartPortStr)) {
		connect_options.startPort = atoi(StartPortStr.c_str());
	}
	arg_parser.Get("LadderServer", connect_options.serverAddress);
	std::string CompOpp;
	if (arg_parser.Get("ComputerOpponent", CompOpp))
	{
		connect_options.computerOpponent = true;
		std::string CompRace;
		if (arg_parser.Get("ComputerRace", CompRace))
		{
			connect_options.computerRace = getRaceFromString(CompRace);
		}
		std::string CompDiff;
		if (arg_parser.Get("ComputerDifficulty", CompDiff))
		{
			connect_options.computerDifficulty = getDifficultyFromString(CompDiff);
		}
		std::string map;
		if (arg_parser.Get("Map", map))
		{
			connect_options.map = map;
		}
		else {
			connect_options.map = kDefaultMap;
		}
	}
	else
	{
		connect_options.computerOpponent = false;
	}
	arg_parser.Get("OpponentId", connect_options.opponentId);
}

static void runBot(int argc, char *argv[], sc2::Agent *Agent, sc2::Race race)
{
	ConnectionOptions Options;
	parseArguments(argc, argv, Options);

	sc2::Coordinator coordinator;

	int num_agents;
	if (Options.computerOpponent) {
		num_agents = 1;
		coordinator.SetParticipants({
			CreateParticipant(race, Agent),
			CreateComputer(Options.computerRace, Options.computerDifficulty)
			});


		coordinator.LoadSettings(1, argv);

		// my code
		//coordinator.SetRealtime(true); // real-time simulations
		//coordinator.SetStepSize(15);   // super speedy simulations

		coordinator.LaunchStarcraft();
		coordinator.StartGame(Options.map);
	}
	else {
		num_agents = 2;
		coordinator.SetParticipants({
			CreateParticipant(race, Agent),
			});
		// Start the game.
		std::cout << "Connecting to port " << Options.gamePort << std::endl;
		coordinator.Connect(Options.gamePort);
		coordinator.SetupPorts(num_agents, Options.startPort, false);
		// Step forward the game simulation.
		coordinator.JoinGame();
		std::cout << " Successfully joined game" << std::endl;
	}

	coordinator.SetTimeoutMS(10000);
	while (coordinator.Update()) {
	}
}
