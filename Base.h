#pragma once

#include "sc2api/sc2_api.h"

class Base {
public:
	Base(sc2::Point2D location_);
	Base(float x, float y);
	void addBuildArea(sc2::Point2D location_);
	void addBuildArea(float x, float y);
	void addDefendPoint(sc2::Point2D location_);
	void addDefendPoint(float x, float y);
	int getNumBuildAreas();
	int getNumDefendPoints();
	sc2::Point2D getRandomBuildArea();
	sc2::Point2D getRandomDefendPoint();
	void setActive(bool flag = true);
	bool isActive();
	sc2::Point2D getBuildArea(int index);
	sc2::Point2D getDefendPoint(int index);
	sc2::Point2D getTownhall();
	void setRallyPoint(float x, float y);
	void setMeleeRallyPoint(float x, float y);
	sc2::Point2D getRallyPoint();

	sc2::Point2D getMeleeRallyPoint();

private:
	sc2::Point2D location_townhall;
	std::vector<sc2::Point2D> defend_points;
	std::vector<sc2::Point2D> build_areas;
	bool active;
	sc2::Point2D rally_point;
	sc2::Point2D melee_rally_point;
};