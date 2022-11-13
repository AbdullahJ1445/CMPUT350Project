#pragma once

#include "sc2api/sc2_api.h"

class Base {
public:
	Base(sc2::Point2D location_);
	Base(float x, float y);
	void add_build_area(sc2::Point2D location_);
	void add_build_area(float x, float y);
	void add_defend_point(sc2::Point2D location_);
	void add_defend_point(float x, float y);
	int get_num_build_areas();
	int get_num_defend_points();
	sc2::Point2D get_random_build_area();
	sc2::Point2D get_random_defend_point();
	void set_active(bool flag=true);
	bool is_active();
	sc2::Point2D get_build_area(int index);
	sc2::Point2D get_defend_point(int index);
	sc2::Point2D get_townhall();

private:
	sc2::Point2D location_townhall;
	std::vector<sc2::Point2D> defend_points;
	std::vector<sc2::Point2D> build_areas;
	bool active;
};