#include "Base.h"

Base::Base(sc2::Point2D location_) {
	// stores the details of a base
	location_townhall = location_;
	active = false;
	rally_point = location_;
}

Base::Base(float x, float y) : Base(sc2::Point2D(x,y)) {}

void Base::setRallyPoint(float x, float y) {
	sc2::Point2D location_(x, y);
	rally_point = location_;
}

sc2::Point2D Base::getRallyPoint() {
	return rally_point;
}

void Base::addBuildArea(sc2::Point2D location_) {
	// adds a build area to the base, with radius of 12.0
	build_areas.push_back(location_);
}

void Base::addBuildArea(float x, float y) {
	addBuildArea(sc2::Point2D(x, y));
}

void Base::addDefendPoint(sc2::Point2D location_) {
	// adds a defend point to the base, should use radius of 7.0
	defend_points.push_back(location_);
}

void Base::addDefendPoint(float x, float y) {
	addDefendPoint(sc2::Point2D(x, y));
}

sc2::Point2D Base::getBuildArea(int index) {
	// Get the build area with the specified index, or the best alternative
	int num_build_areas = getNumBuildAreas();
	int num_defend_points = getNumDefendPoints();
	if (num_build_areas == 0) {
		// if the base has no build areas, find an alternative
		if (num_defend_points > 0) {
			// return a defend point if one exists
			return getRandomDefendPoint();
		}
		// if all else fails, return the townhall location
		return location_townhall;	
	}
	// if index is higher than last, get the last index instead
	if (index >= num_build_areas || index < 0) {
		index = num_build_areas - 1;
	}

	return build_areas[index];
}

sc2::Point2D Base::getDefendPoint(int index) {
	// Get the defend point with the specified index, or the best alternative

	int num_build_areas = getNumBuildAreas();
	int num_defend_points = getNumDefendPoints();
	if (num_defend_points == 0) {
		// if the base has no defend points, find an alternative
		if (num_build_areas > 0) {
			// return a build area if one exists
			return getRandomBuildArea();
		}
		// if all else fails, return the townhall location
		return location_townhall;
	}
	// if index is higher than last, get the last index instead
	if (index > num_defend_points || index < 0)
		index = num_defend_points - 1;

	return defend_points[index];
}

sc2::Point2D Base::getTownhall() {
	return location_townhall;
}
int Base::getNumBuildAreas() { 
	// return the number of build areas in the base
	return(build_areas.size());
}

int Base::getNumDefendPoints() { 
	// return the number of defend areas in the base
	return(defend_points.size());
}

sc2::Point2D Base::getRandomBuildArea() {
	// get a random build area within the base

	int num_build_areas = getNumBuildAreas();
	int num_defend_points = getNumDefendPoints();
	if (num_build_areas == 0) {
		// if the base has no build areas, find an alternative
		if (num_defend_points > 0) {
			// return a defend point if one exists
			return defend_points[sc2::GetRandomInteger(0, num_defend_points - 1)];
		}
		// if all else fails, return the townhall location
		return location_townhall;
	}
	// if index is higher than last, get the last index instead
	return build_areas[sc2::GetRandomInteger(0, num_build_areas - 1)];
}

sc2::Point2D Base::getRandomDefendPoint() {
	// get a random defend points within the base

	int num_build_areas = getNumBuildAreas();
	int num_defend_points = getNumDefendPoints();
	if (num_defend_points == 0) {
		// if the base has no build areas, find an alternative
		if (num_build_areas > 0) {
			// return a defend point if one exists
			return build_areas[sc2::GetRandomInteger(0, num_build_areas - 1)];
		}
		// if all else fails, return the townhall location
		return location_townhall;
	}
	// if index is higher than last, get the last index instead
	return defend_points[sc2::GetRandomInteger(0, num_defend_points - 1)];
}

void Base::setActive(bool flag) {
	// sets the base as active (optionally use flag=false to deactivate)
	active = flag;
}

bool Base::isActive() {
	return active;
}