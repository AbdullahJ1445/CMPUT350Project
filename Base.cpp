#include "Base.h"

Base::Base(sc2::Point2D location_) {
	// stores the details of a base
	location_townhall = location_;
}

Base::Base(float x, float y) : Base(sc2::Point2D(x,y)) {}

void Base::add_build_area(sc2::Point2D location_) { 
	// adds a build area to the base, with radius of 12.0
	build_areas.push_back(location_);
}

void Base::add_build_area(float x, float y) {
	add_build_area(sc2::Point2D(x, y));
}

void Base::add_defend_point(sc2::Point2D location_) {
	// adds a defend point to the base, should use radius of 7.0
	defend_points.push_back(location_);
}

void Base::add_defend_point(float x, float y) {
	add_defend_point(sc2::Point2D(x, y));
}

sc2::Point2D Base::get_build_area(int index) {
	// Get the build area with the specified index, or the best alternative

	int num_build_areas = get_num_build_areas();
	int num_defend_points = get_num_defend_points();
	if (num_build_areas == 0) {
		// if the base has no build areas, find an alternative
		if (num_defend_points > 0) {
			// return a defend point if one exists
			return get_random_defend_point();
		}
		// if all else fails, return the townhall location
		return location_townhall;	
	}
	// if index is higher than last, get the last index instead
	if (index > num_build_areas || index < 0)
		index = num_build_areas - 1;

	return build_areas[index];
}

sc2::Point2D Base::get_defend_point(int index) {
	// Get the defend point with the specified index, or the best alternative

	int num_build_areas = get_num_build_areas();
	int num_defend_points = get_num_defend_points();
	if (num_defend_points == 0) {
		// if the base has no defend points, find an alternative
		if (num_build_areas > 0) {
			// return a build area if one exists
			return get_random_build_area();
		}
		// if all else fails, return the townhall location
		return location_townhall;
	}
	// if index is higher than last, get the last index instead
	if (index > num_defend_points || index < 0)
		index = num_defend_points - 1;

	return defend_points[index];
}

sc2::Point2D Base::get_townhall() {
	return location_townhall;
}
int Base::get_num_build_areas() { 
	// return the number of build areas in the base
	return(build_areas.size());
}

int Base::get_num_defend_points() { 
	// return the number of defend areas in the base
	return(defend_points.size());
}

sc2::Point2D Base::get_random_build_area() {
	// get a random build area within the base
	return sc2::GetRandomEntry<std::vector<sc2::Point2D>>(build_areas);
}

sc2::Point2D Base::get_random_defend_point() {
	// get a random build area within the base
	return sc2::GetRandomEntry<std::vector<sc2::Point2D>>(defend_points);
}

void Base::set_active(bool flag) {
	// sets the base as active (optionally use flag=false to deactivate)
	active = flag;
}