#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "BasicSc2Bot.h"
#include "LadderInterface.h"

using namespace sc2;

class Human : public sc2::Agent {
public:
	void OnGameStart() final {
		Debug()->DebugTextOut("Testing with Human");
		Debug()->SendDebug();

	}
};

class Bot : public Agent {
public:
	virtual void OnGameStart() final {
		std::cout << "LET THE GAME BEGIN" << std::endl;
		initVariables();
		float rx = GetRandomScalar();
		float ry = GetRandomScalar();
		start_location = Observation()->GetStartLocation();
		ChooseProxyWorker();
		army_rally = Point2D(start_location.x + rx * 20.0f, start_location.y + ry * 20.0f);
		std::cout << "Rally Point: " << army_rally.x << ", " << army_rally.y << std::endl;
	}

	virtual void OnStep() final {
		TryBuildPylon();
		TryBuildGateway();
		TryBuildCybernetics();
		ManageProxyBase();
		const ObservationInterface* observation = Observation();
		int minerals = observation->GetMinerals();
		if (observation->GetGameLoop() % 1000 == 0) {
			std::cout << ".";
			//std::cout << "Minerals: " << minerals << std::endl;
		}
		// std::cout << Observation()->GetGameLoop() << std::endl;
	}

	virtual void OnUnitIdle(const Unit* unit) final {
		switch (unit->unit_type.ToType()) {
			case UNIT_TYPEID::PROTOSS_NEXUS: {
				int probe_cost;
				probe_cost = Observation()->GetUnitTypeData()[84].mineral_cost; // 84 is index of protoss probe
				if ((Observation()->GetMinerals() >= probe_cost) && NeedMoreWorkers(unit))
					Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
				break;
			}
			case UNIT_TYPEID::PROTOSS_GATEWAY: {
				int zealot_cost;
				zealot_cost = Observation()->GetUnitTypeData()[73].mineral_cost; // 73 is index of protoss zealot
				if ((Observation()->GetMinerals() >= zealot_cost))
					Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_ZEALOT);
				break;
			}
			case UNIT_TYPEID::PROTOSS_STARGATE: {
				int voidray_cost;
				voidray_cost = Observation()->GetUnitTypeData()[80].mineral_cost; // 80 is index of protoss void ray
				if ((Observation()->GetMinerals() >= voidray_cost))
					Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_VOIDRAY);
				break;
			}
			case UNIT_TYPEID::PROTOSS_ZEALOT: {
				Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, army_rally);
				break;
			}
			case UNIT_TYPEID::PROTOSS_VOIDRAY: {
				Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, enemy_location);
				break;
			}
			case UNIT_TYPEID::PROTOSS_PROBE: {
				if (unit == &proxy_worker)
					std::cout << "PROXY WORKER IDLED" << std::endl;
					break;
				const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
				if (!mineral_target) {
					break;
				}
				Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
				break;
			}
			default: {
				break;
			}
		}
	}

private:
	Point2D start_location;
	Point2D army_rally;
	Point2D choke_point_1;
	Point2D choke_point_2;
	Point2D proxy_location;
	Point2D enemy_location;
	Unit proxy_worker;

	int player_start_id;
	int enemy_start_id;
	std::string map_name;
	int map_index; // 1 = CactusValleyLE,  2 = BelShirVestigeLE,  3 = ProximaStationLE

	bool ChooseProxyWorker() {
		Units units = Observation()->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			std::cout << "looking for a probe.. ";
			if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
				std::cout << "probe found" << std::endl;
				proxy_worker = *unit;
				Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, proxy_location);
				break;
			}
		}
		if (&proxy_worker == NULL)
			return false;
		std::cout << "Proxy Worker chosen! [" << proxy_worker.unit_type.to_string() << "]" << std::endl;
		if (proxy_location != Point2D(NULL, NULL)) {
			Actions()->UnitCommand(&proxy_worker, ABILITY_ID::ATTACK, proxy_location);
		}
		return true;
	}

	bool TryBuildGateway() {
		const ObservationInterface* observation = Observation();
		int minerals = observation->GetMinerals();
		int food = observation->GetFoodCap();
		if (minerals < 150)
			return false;
		if (food < 12)
			return false;
		if (food < 20 && CountUnits(UNIT_TYPEID::PROTOSS_GATEWAY) < 2) {
			return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY);
		}
		if (food < 26 && CountUnits(UNIT_TYPEID::PROTOSS_GATEWAY) < 3) {
			return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY);
		}
		if (food < 32 && CountUnits(UNIT_TYPEID::PROTOSS_GATEWAY) < 3) {
			return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY);
		}
		return false;
	}

	bool TryBuildCybernetics() {
		const ObservationInterface* observation = Observation();
		int minerals = observation->GetMinerals();
		if (minerals < 200)
			return false;
		if (CountUnits(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0) {
			return TryBuildStructure(ABILITY_ID::BUILD_CYBERNETICSCORE);
		}
		return false;
	}

	bool NeedMoreWorkers(const Unit* unit) {
		Units units = Observation()->GetUnits(Unit::Alliance::Ally);
		int non_mineral_probes = 0;
		for (const auto& unit : units) {
			if (unit == &proxy_worker)
				non_mineral_probes++;
		}

		return CountUnits(UNIT_TYPEID::PROTOSS_PROBE) - non_mineral_probes < (CountMineralsNearNexus(unit->pos) * 2);
	}

	int CountUnits(UNIT_TYPEID u) {
		const ObservationInterface* observation = Observation();
		int count = 0;

		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			if (unit->unit_type == u) {
				count++;
			}
		}
		return count;
	}

	const Unit* FindNearestMineralPatch(const Point2D& start) {
		Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
		float distance = std::numeric_limits<float>::max();
		const Unit* target = nullptr;
		for (const auto& u : units) {
			if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
				float d = DistanceSquared2D(u->pos, start);
				if (d < distance) {
					distance = d;
					target = u;
				}
			}
		}
		return target;
	}

	int CountMineralsNearNexus(const Point2D& start) {
		Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
		float RANGE_FOR_MINERALS = 18.0;
		int count = 0;
		for (const auto& u : units) {
			UNIT_TYPEID u_type = u->unit_type;
			if (u_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD || u_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD450 || u_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750
				|| u_type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD || u_type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750) {
				if (sqrt(DistanceSquared2D(u->pos, start)) < RANGE_FOR_MINERALS) {
					count++;
				}
			}
		}
		std::cout << count << " minerals near nexus" << std::endl;
		return count;
	}


	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::PROTOSS_PROBE) {
		const ObservationInterface* observation = Observation();

		// if a unit is already building a supply structure of this type, do nothing.
		// Get a probe to build the structure
		const Unit* unit_to_build = nullptr;
		Units units = observation->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			for (const auto& order : unit->orders) {
				if (order.ability_id == ability_type_for_structure) {
					return false;
				}
				if (unit == &proxy_worker)
					return false;
			}
			if (unit->unit_type == unit_type) {
				unit_to_build = unit;
			}
		}

		float rx = GetRandomScalar();
		float ry = GetRandomScalar();

		Actions()->UnitCommand(unit_to_build, ability_type_for_structure,
			Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));

		return true;
	}

	bool ManageProxyBase() {
		const ObservationInterface* observation = Observation();
		int proxy_pylons = CountUnitsNearPoint(UNIT_TYPEID::PROTOSS_PYLON, proxy_location);
		if (proxy_pylons == 0)
		{
			return TryBuildProxyStructure(ABILITY_ID::BUILD_PYLON);
		}
		int num_stargates = CountUnitsNearPoint(UNIT_TYPEID::PROTOSS_STARGATE, proxy_location);
		int num_cybernetics = CountUnits(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE);
		if (num_cybernetics == 0)
			return false;
		int minerals = observation->GetMinerals();
		int food = observation->GetFoodCap();
		if (num_stargates == 0) {
			return TryBuildProxyStructure(ABILITY_ID::BUILD_STARGATE);
		}
		if (num_stargates == 1 && food > 22) {
			return TryBuildProxyStructure(ABILITY_ID::BUILD_STARGATE);
		}

	}

	int CountUnitsNearPoint(UNIT_TYPEID unit_type, Point2D point, float range=18.0f) {
		const ObservationInterface* observation = Observation();
		Units units = observation->GetUnits(Unit::Alliance::Self);
		int count = 0;
		for (const auto& unit : units) {
			if (unit->unit_type == unit_type)
				if (Distance2D(unit->pos, point) < range)
					count++;
		}
		return count;
	}

	bool TryBuildProxyStructure(ABILITY_ID ability_type_for_structure) {
		float rx = GetRandomScalar();
		float ry = GetRandomScalar();

		if (ability_type_for_structure == ABILITY_ID::BUILD_PYLON) {
			Actions()->UnitCommand(&proxy_worker, ability_type_for_structure,
				Point2D(proxy_location.x, proxy_location.y));
		}
		Actions()->UnitCommand(&proxy_worker, ability_type_for_structure,
			Point2D(proxy_location.x + rx * 4.0f, proxy_location.y + ry * 4.0f));

		return true;
	}

	bool TryBuildPylon() {
		const ObservationInterface* observation = Observation();

		// always build a pylon if you have none, and have more than 150 minerals
		if (CountUnits(UNIT_TYPEID::PROTOSS_PYLON) < 1 && observation->GetMinerals() >= 150)
			return TryBuildStructure(ABILITY_ID::BUILD_PYLON);

		// if we are not supply capped, don't build a pylon.
		if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
			return false;

		// Try and build a pylon. Find a random probe and give it the order.
		return TryBuildStructure(ABILITY_ID::BUILD_PYLON);
	}

	void initVariables() {
		const ObservationInterface* observation = Observation();
		map_name = observation->GetGameInfo().map_name;
		
		if (map_name == "Proxima Station LE")
			map_index = 3; else
			if (map_name == "Bel'Shir Vestige LE (Void)" || map_name == "Bel'Shir Vestige LE")
				map_index = 2; else
				if (map_name == "Cactus Valley LE")
					map_index = 1; else
					map_index = 0;
		
		player_start_id = getPlayerIDForMap(map_index, observation->GetStartLocation());
		initLocations(map_index, player_start_id);

		std::cout << "Map Name: " << map_name << std::endl;
		std::cout << "Player Start ID: " << player_start_id << std::endl;
	}

	void initLocations(int map_index, int p_id) {
		switch (map_index) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			switch (p_id) {
			case 1:
				choke_point_1 = Point2D(146.0, 119.0);
				choke_point_2 = Point2D(137.0, 90.0);
				proxy_location = Point2D(28.0, 56.0);
				enemy_location = Point2D(62.5, 28.5);
				break;
			case 2:
				choke_point_1 = Point2D(54.0, 49.0);
				choke_point_2 = Point2D(63.0, 78.0);
				proxy_location = Point2D(172.0, 112.0);
				enemy_location = Point2D(137.5, 139.5);
				break;
			}
			break;
		}
	}

	int getPlayerIDForMap(int map_index, Point2D location) {
		location = getNearestStartLocation(location);
		int p_id = 0;
		switch (map_index) {
		case 1:
			// Cactus Valley LE
			if (location == Point2D(33.5, 158.5)) {
				// top left
				p_id = 1;
			}
			if (location == Point2D(158.5, 158.5)) {
				// top right
				p_id = 2;
			}
			if (location == Point2D(158.5, 158.5)) {
				// bottom right
				p_id = 3;
			}
			if (location == Point2D(33.5, 33.5)) {
				// bottom left
				p_id = 4;
			}
			break;
		case 2:
			// Bel'Shir Vestige LE
			if (location == Point2D(114.5, 25.5)) {
				// bottom right
				p_id = 1;
			}
			if (location == Point2D(29.5, 134.5)) {
				// top left
				p_id = 2;
			}
			break;
		case 3:
			// Proxima Station LE
			if (location == Point2D(137.5, 139.5)) {			
				// top right
				p_id = 1;
			}
			if (location == Point2D(62.5, 28.5)) {
				// bottom left
				p_id = 2;
			}
			break;
		}
		return p_id;
	}

	Point2D getNearestStartLocation(Point2D spot) {
		// Get the nearest Start Location from a given point
		float nearest_distance = 10000.0f;
		Point2D nearest_point;
		
		for (auto& iter : Observation()->GetGameInfo().start_locations) {
			float dist = Distance2D(spot, iter);
			if (dist <= nearest_distance) {
				nearest_distance = dist;
				nearest_point = iter;
			}
		}
		return nearest_point;
	}
};

int main(int argc, char* argv[]) {
	//RunBot(argc, argv, new BasicSc2Bot(), sc2::Race::Protoss);
	Coordinator coordinator;
	coordinator.LoadSettings(argc, argv);
	const char* kProximaStationLE = "Ladder/ProximaStationLE.SC2Map";
	const char* kCactusValleyLE = "Ladder/CactusValleyLE.SC2Map";
	bool human_player = false;

	Bot bot;
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
	//coordinator.StartGame(kMapBelShirVestigeLE);
	coordinator.StartGame(kProximaStationLE);

	while (coordinator.Update()) {
	}

	return 0;
}