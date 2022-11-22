#include "MobHandler.h"
#include "Mob.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"

MobHandler::MobHandler(BasicSc2Bot* agent) {
    this->agent = agent;
}

void MobHandler::set_mob_idle(Mob* mob_, bool is_true) {
    // set a mob as idle

	Mob* mob = &getMob(mob_->unit); // ensure we are pointing to the mob in our storage
	if (is_true) {
		mob_->set_flag(FLAGS::IS_IDLE);
		mob_->remove_flag(FLAGS::IS_BUILDING_STRUCTURE);
		set_mob_busy(mob_, false);
		idle_mobs.insert(mob);
	}
	else {
		mob_->remove_flag(FLAGS::IS_IDLE);
		idle_mobs.erase(mob);
	}
}

void MobHandler::set_mob_busy(Mob* mob_, bool is_true) {
	// set a mob as busy
	// a mob may be neither busy or idle (e.g. a worker harvesting minerals will be available to build)

	Mob* mob = &getMob(mob_->unit); // ensure we are pointing to the mob in our storage
	if (is_true) {
		set_mob_idle(mob_, false);
		busy_mobs.insert(mob);
	}
	else {
		busy_mobs.erase(mob);
	}
}

bool MobHandler::addMob(Mob mob_) {
	// add a mob to the game

	if (mob_exists(mob_.unit))
		return false;

	mobs_storage.emplace_back(std::make_unique<Mob>(mob_));
	int size = mobs_storage.size();

	mobs.insert(mobs_storage.back().get());
	mob_by_tag[mob_.unit.tag] = mobs_storage.back().get();

	return true;
}

bool MobHandler::mob_exists(const sc2::Unit& unit) {
	// check whether mob exists in storage

	return mob_by_tag[unit.tag];
}

Mob& MobHandler::getMob(const sc2::Unit& unit) {
	// get the correct mob from storage

	return *mob_by_tag[unit.tag];
}

void MobHandler::mobDeath(Mob* mob_)
{
	Mob* mob = &getMob(mob_->unit);
	set_mob_idle(mob, false);
	set_mob_busy(mob, false);
	for (auto f : mob->get_flags()) {
		mob->remove_flag(f);
	}
	mobs.erase(mob);
}

std::unordered_set<Mob*> MobHandler::getIdleWorkers() {
	return filter_by_flag(idle_mobs, FLAGS::IS_WORKER);
}

std::unordered_set<Mob*> MobHandler::filter_by_flag(std::unordered_set<Mob*> mobs_set, FLAGS flag, bool is_true) {
	// filter a vector of Mob* by the given flag

	std::unordered_set<Mob*> filtered_mobs;

	std::copy_if(mobs_set.begin(), mobs_set.end(), std::inserter(filtered_mobs, filtered_mobs.begin()),
		[flag, is_true](Mob* m) { return m->has_flag(flag) == is_true; });

	return filtered_mobs;
}

std::unordered_set<Mob*> MobHandler::filter_by_flags(std::unordered_set<Mob*> mobs_set, std::unordered_set<FLAGS> flag_list, bool is_true) {
	// filter a vector of Mob* by several flags

	std::unordered_set<Mob*> filtered_mobs = mobs_set;
	for (FLAGS f : flag_list) {
		filtered_mobs = filter_by_flag(filtered_mobs, f, is_true);
	}
	return filtered_mobs;
}

std::unordered_set<Mob*> MobHandler::get_mobs() {
	return mobs;
}

std::unordered_set<Mob*> MobHandler::get_idle_mobs() {
    return idle_mobs;
}

std::unordered_set<Mob*> MobHandler::get_busy_mobs() {
	return busy_mobs;
}

std::unordered_set<Mob*> MobHandler::getMobGroupByName(std::string mobName) {
	return mob_group_by_name[mobName];
}
