#pragma once

#include "Directive.h"
#include "Triggers.h"
#include "sc2api/sc2_api.h"

Trigger::TriggerCondition::TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, int cond_value_) {
	cond_type = cond_type_;
	cond_value = cond_value_;
	agent = agent_;
	debug = false;
}

Trigger::TriggerCondition::TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_) {
	assert(cond_type_ == COND::HAS_ABILITY_READY);
	cond_type = cond_type_;
	ability_id = ability_id_;
	is_true = is_true_;
	agent = agent_;
	unit_of_type = unit_of_type_;
	debug = false;
}

Trigger::TriggerCondition::TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_) {
	assert(cond_type_ == COND::HAVE_UPGRADE);
	cond_type = cond_type_;
	upgrade_id = upgrade_id_;
	is_true = is_true_;
	agent = agent_;
	debug = false;
}

Trigger::TriggerCondition::TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_) {
	assert(cond_type_ == COND::MIN_UNIT_WITH_FLAGS ||
		cond_type_ == COND::MAX_UNIT_WITH_FLAGS);
	cond_type = cond_type_;
	cond_value = cond_value_;
	filter_flags = flags_;
	agent = agent_;
	debug = false;
}

Trigger::TriggerCondition::TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_, sc2::Point2D location_, float radius_) {
	assert(cond_type_ == COND::MIN_UNIT_WITH_FLAGS_NEAR_LOCATION || 
		cond_type == COND::MAX_UNIT_WITH_FLAGS_NEAR_LOCATION);
	cond_type = cond_type_;
	cond_value = cond_value_;
	filter_flags = flags_;
	agent = agent_;
	distance_squared = pow(radius_, 2);
	location_for_counting_units = location_;
	debug = false;
}

Trigger::TriggerCondition::TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_) {
	assert(cond_type_ == COND::MAX_UNIT_OF_TYPE || cond_type_ == COND::MIN_UNIT_OF_TYPE || 
		cond_type_ == COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION || cond_type_ == COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION ||
		cond_type_ == COND::MIN_UNIT_OF_TYPE_TOTAL || cond_type_ == COND::MAX_UNIT_OF_TYPE_TOTAL);
	cond_type = cond_type_;
	cond_value = cond_value_;
	unit_of_type = unit_of_type_;
	agent = agent_;
	debug = false;
}

Trigger::TriggerCondition::TriggerCondition(BasicSc2Bot* agent_, COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_) {
	assert(cond_type_ == COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION || cond_type_ == COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION ||
	cond_type_ == COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION || cond_type_ == COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION ||
	cond_type_ == COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION || cond_type_ == COND::MIN_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION);
	cond_type = cond_type_;
	cond_value = cond_value_;
	unit_of_type = unit_of_type_;
	location_for_counting_units = location_;
	distance_squared = pow(radius_, 2);
	agent = agent_;
	debug = false;
}

bool Trigger::TriggerCondition::is_met(const sc2::ObservationInterface* obs) {

	// assign equivalent_type for the same units that might have an alternate ID
	sc2::UNIT_TYPEID equivalent_type = unit_of_type;
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_REFINERY) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_REFINERYRICH;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::TERRAN_REFINERYRICH) {
		equivalent_type = sc2::UNIT_TYPEID::TERRAN_REFINERY;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
		equivalent_type = sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH) {
		equivalent_type = sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::ZERG_EXTRACTOR) {
		equivalent_type = sc2::UNIT_TYPEID::ZERG_EXTRACTORRICH;
	}
	if (unit_of_type == sc2::UNIT_TYPEID::ZERG_EXTRACTORRICH) {
		equivalent_type = sc2::UNIT_TYPEID::ZERG_EXTRACTOR;
	}

	switch (cond_type) {
	case COND::MIN_MINERALS:
		return obs->GetMinerals() >= cond_value;
	case COND::MIN_GAS:
		return obs->GetVespene() >= cond_value;
	case COND::MIN_TIME:
		return obs->GetGameLoop() >= cond_value;
	case COND::MIN_FOOD:
		return obs->GetFoodCap() - obs->GetFoodUsed() >= cond_value;
	case COND::MIN_FOOD_USED:
		return obs->GetFoodUsed() >= cond_value;
	case COND::MIN_FOOD_CAP:
		return obs->GetFoodCap() >= cond_value;
	case COND::MAX_MINERALS:
		return obs->GetMinerals() <= cond_value;
	case COND::MAX_GAS:
		return obs->GetVespene() <= cond_value;
	case COND::MAX_TIME:
		return obs->GetGameLoop() <= cond_value;
	case COND::MAX_FOOD:
		return obs->GetFoodCap() - obs->GetFoodUsed() <= cond_value;
	case COND::MAX_FOOD_USED:
		return obs->GetFoodUsed() <= cond_value;
	case COND::MAX_FOOD_CAP:
		return obs->GetFoodCap() <= cond_value;
	case COND::MIN_MINERALS_MINED:
		// not implemented
		return false;
	case COND::MAX_MINERALS_MINED:
		// not implemented
		return false;
	case COND::MIN_UNIT_WITH_FLAGS:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->mobH->filterByFlags(agent->mobH->getMobs(), filter_flags);
		int num_units = mobs.size();

		// output to debug for checking conditions which are failing
		if (debug && num_units < cond_value) {
			std::cout << "MIN_W_F(" << num_units << ">=" << cond_value << ") ";
		}
		return (num_units >= cond_value);
	}
	case COND::MAX_UNIT_WITH_FLAGS:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->mobH->filterByFlags(agent->mobH->getMobs(), filter_flags);
		int num_units = mobs.size();
		
		// output to debug for checking conditions which are failing
		if (debug && num_units > cond_value) {
			std::cout << "MAX_W_F(" << num_units << "<=" << cond_value << ") ";
		}
		return (num_units <= cond_value);
	}
	case COND::MIN_UNIT_WITH_FLAGS_NEAR_LOCATION:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->mobH->filterByFlags(agent->mobH->getMobs(), filter_flags);
		std::unordered_set<Mob*> filtered_mobs;
		std::copy_if(mobs.begin(), mobs.end(), std::inserter(filtered_mobs, filtered_mobs.begin()),
			[this](Mob* m) { return (
				sc2::DistanceSquared2D(m->unit.pos, location_for_counting_units) <= distance_squared); 
			});
		int num_units = filtered_mobs.size();

		// output to debug for checking conditions which are failing
		if (debug && num_units < cond_value) {
			std::cout << "MIN_W_F_NL(" << num_units << ">=" << cond_value << ") ";
		}

		return (num_units >= cond_value);
	}
	case COND::MAX_UNIT_WITH_FLAGS_NEAR_LOCATION:
	{
		std::unordered_set<Mob*> mobs;
		mobs = agent->mobH->filterByFlags(agent->mobH->getMobs(), filter_flags);
		std::unordered_set<Mob*> filtered_mobs;
		std::copy_if(mobs.begin(), mobs.end(), std::inserter(filtered_mobs, filtered_mobs.begin()),
			[this](Mob* m) { return (
				sc2::DistanceSquared2D(m->unit.pos, location_for_counting_units) <= distance_squared);
			});
		int num_units = filtered_mobs.size();
		
		// output to debug for checking conditions which are failing
		if (debug && num_units > cond_value) {
			std::cout << "MAX_W_F_NL(" << num_units << "<=" << cond_value << ") ";
		}
		
		return (num_units <= cond_value);
	}
	case COND::BASE_IS_ACTIVE:
		if (agent->locH->bases.size() <= cond_value || cond_value < 0)
			return false;
		return agent->locH->bases[cond_value].isActive();
	case COND::HAVE_UPGRADE:
		return agent->haveUpgrade(upgrade_id) == is_true;
	case COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION:
	{
		const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		std::vector<const sc2::Unit*> filtered_units;
		std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
			[this](const sc2::Unit* u) { return (u->unit_type == unit_of_type) && (u->build_progress < 1.0 && u->build_progress > 0); });
		int num_units = filtered_units.size();

		// output to debug for checking conditions which are failing
		if (debug && num_units < cond_value) {
			std::cout << "MIN_UT_UC(" << num_units << ">=" << cond_value << ") ";
		}

		return (num_units >= cond_value);
	}
	case COND::MIN_UNIT_OF_TYPE_TOTAL:
		// include both under construction and constructed
	{
		const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		std::vector<const sc2::Unit*> filtered_units;
		std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
			[this](const sc2::Unit* u) { return (u->unit_type == unit_of_type) && (u->build_progress > 0); });
		int num_units = filtered_units.size();

		// output to debug for checking conditions which are failing
		if (debug && num_units < cond_value) {
			std::cout << "MIN_UTT(" << num_units << ">=" << cond_value << ") ";
		}

		return (num_units >= cond_value);
	}
	case COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION:
		{
			const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
			std::vector<const sc2::Unit*> filtered_units;
			std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
				[this, equivalent_type](const sc2::Unit* u) { return (u->unit_type == unit_of_type || u->unit_type == equivalent_type) && (u->build_progress < 1.0 && u->build_progress > 0); });
			int num_units = filtered_units.size();
		
			// output to debug for checking conditions which are failing
			if (debug && num_units > cond_value) {
				std::cout << "MAX_UT_UC(" << num_units << "<=" << cond_value << ") ";
			}

			return (num_units <= cond_value);
		}
	case COND::MAX_UNIT_OF_TYPE_TOTAL:
		// include both under construction and constructed
	{
		const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		std::vector<const sc2::Unit*> filtered_units;
		std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
			[this, equivalent_type](const sc2::Unit* u) { return (u->unit_type == unit_of_type || u->unit_type == equivalent_type) && (u->build_progress >= 0); });
		int num_units = filtered_units.size();

		// output to debug for checking conditions which are failing
		if (debug && num_units > cond_value) {
			std::cout << "MAX_UTT(" << num_units << "<=" << cond_value << ")";
		}

		return (num_units <= cond_value);
	}
	case COND::MAX_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION:
		{
			sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
			int num_units = count_if(units.begin(), units.end(),
				[this](const sc2::Unit* u) {
					return (u->unit_type == unit_of_type
						&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared)
						&& (u->build_progress > 0 && u->build_progress < 1.0F);
				});

			// output to debug for checking conditions which are failing
			if (debug && num_units > cond_value) {
				std::cout << "MAX_UT_UC_NL(" << num_units << "<=" << cond_value << ")";
			}

			return (num_units <= cond_value);
		}
	case COND::MAX_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION:
		// include both under construction and constructed
	{
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) {
				return (u->unit_type == unit_of_type
					&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared)
					&& (u->build_progress >= 0);
			});

		// output to debug for checking conditions which are failing
		if (debug && num_units > cond_value) {
			std::cout << "MAX_UTT_NL(" << num_units << "<=" << cond_value << ")";
		}

		return (num_units <= cond_value);
	}
	case COND::MIN_UNIT_OF_TYPE_UNDER_CONSTRUCTION_NEAR_LOCATION:
		{
			sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
			int num_units = count_if(units.begin(), units.end(),
				[this](const sc2::Unit* u) {
					return (u->unit_type == unit_of_type
						&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared)
						&& (u->build_progress > 0 && u->build_progress < 1.0F);
				});

			// output to debug for checking conditions which are failing
			if (debug && num_units < cond_value) {
				std::cout << "MIN_UT_UC_NL(" << num_units << ">=" << cond_value << ")";
			}

			return (num_units >= cond_value);
		}
	case COND::MIN_UNIT_OF_TYPE_TOTAL_NEAR_LOCATION:
		// include both under construction and constructed
	{
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) {
				return (u->unit_type == unit_of_type
					&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared)
					&& (u->build_progress > 0);
			});

		// output to debug for checking conditions which are failing
		if (debug && num_units < cond_value) {
			std::cout << "MIN_UTT_NL(" << num_units << ">=" << cond_value << ")";
		}

		return (num_units >= cond_value);
	}
	case COND::HAS_ABILITY_READY:
		std::unordered_set<Mob*> structures = agent->mobH->filterByFlag(agent->mobH->getMobs(), FLAGS::IS_STRUCTURE);
		bool found_one = false;
		for (auto m : structures) {
			if (agent->canUnitUseAbility(m->unit, ability_id)) {
				found_one = true;
				break;
			}
		}
		return (found_one == is_true);
	}
	
	if (cond_type == COND::MAX_UNIT_OF_TYPE) {
		// only consider units that have completed construction
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);

		int num_units = count_if(units.begin(), units.end(),
			[this, equivalent_type](const sc2::Unit* u) { 
				return (u->unit_type == unit_of_type || u->unit_type == equivalent_type) && (u->build_progress == 1.0); });

		// output to debug for checking conditions which are failing
		if (debug && num_units > cond_value) {
			//std::cout << "MAX_U(" << num_units << "<=" << cond_value << ")";
		}

		return (num_units <= cond_value);
	}
	if (cond_type == COND::MIN_UNIT_OF_TYPE) {
		// only consider units that have completed construction
		const sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		std::vector<const sc2::Unit*> filtered_units;
		std::copy_if(units.begin(), units.end(), std::back_inserter(filtered_units),
			[this, equivalent_type](const sc2::Unit * u) { return (u->unit_type == unit_of_type || u->unit_type == equivalent_type) && (u->build_progress == 1.0); });
		int num_units = filtered_units.size();

		// output to debug for checking conditions which are failing
		if (debug && num_units < cond_value) {
			std::cout << "MIN_U(" << num_units << ">=" << cond_value << ")";
		}

		return (num_units >= cond_value);
	}

	if (cond_type == COND::MAX_UNIT_OF_TYPE_NEAR_LOCATION) {
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) { 
				return (u->unit_type == unit_of_type
					&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared)
					&& (u->build_progress == 1.0);
			});

		// output to debug for checking conditions which are failing
		if (debug && num_units > cond_value) {
			std::cout << "MAX_UT_NL(" << num_units << "<=" << cond_value << ")";
		}

		return (num_units <= cond_value);
	}
	if (cond_type == COND::MIN_UNIT_OF_TYPE_NEAR_LOCATION) {
		sc2::Units units = obs->GetUnits(sc2::Unit::Alliance::Self);
		int num_units = count_if(units.begin(), units.end(),
			[this](const sc2::Unit* u) {
				return (u->unit_type == unit_of_type
					&& sc2::DistanceSquared2D(u->pos, location_for_counting_units) < distance_squared) 
					&& (u->build_progress == 1.0);
			});

		// output to debug for checking conditions which are failing
		if (debug && num_units < cond_value) {
			std::cout << "MIN_UT_NL(" << num_units << ">=" << cond_value << ")";
		}

		return (num_units >= cond_value);
	}
	return false;
}

void Trigger::TriggerCondition::setDebug(bool is_true) {
	debug = is_true;
}

Trigger::Trigger(BasicSc2Bot* agent_) {
	agent = agent_;
	debug = false;
};

void Trigger::addCondition(TriggerCondition tc_) {
	conditions.push_back(tc_);
}

void Trigger::addCondition(COND cond_type_, int cond_value_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_);
	if (debug) {
		tc_.setDebug(true);
	}
	conditions.push_back(tc_);
}

void Trigger::addCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, unit_of_type_);
	if (debug) {
		tc_.setDebug(true);
	}
	conditions.push_back(tc_);
}

void Trigger::addCondition(COND cond_type_, sc2::UNIT_TYPEID unit_of_type_, sc2::ABILITY_ID ability_id_, bool is_true_) {
	TriggerCondition tc_(agent, cond_type_, unit_of_type_, ability_id_, is_true_);
	if (debug) {
		tc_.setDebug(true);
	}
	conditions.push_back(tc_);
}

void Trigger::addCondition(COND cond_type_, sc2::UPGRADE_ID upgrade_id_, bool is_true_) {
	TriggerCondition tc_(agent, cond_type_, upgrade_id_, is_true_);
	if (debug) {
		tc_.setDebug(true);
	}
	conditions.push_back(tc_);
}

void Trigger::addCondition(COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, flags_);
	if (debug) {
		tc_.setDebug(true);
	}
	conditions.push_back(tc_);
}

void Trigger::addCondition(COND cond_type_, int cond_value_, std::unordered_set<FLAGS> flags_, sc2::Point2D location_, float radius_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, flags_, location_, radius_);
	if (debug) {
		tc_.setDebug(true);
	}
	conditions.push_back(tc_);
}

void Trigger::addCondition(COND cond_type_, int cond_value_, sc2::UNIT_TYPEID unit_of_type_, sc2::Point2D location_, float radius_) {
	TriggerCondition tc_(agent, cond_type_, cond_value_, unit_of_type_, location_, radius_);
	if (debug) {
		tc_.setDebug(true);
	}
	conditions.push_back(tc_);
}


bool Trigger::checkConditions() {
	// Iterate through all conditions and return false if any are not met.
	// Otherwise return true.
	const sc2::ObservationInterface* obs = agent->Observation();
	for (auto c_ : conditions) {
		if (!c_.is_met(obs))
			return false;
	}
	if (debug) {
		std::cout << "[CONDS_PASSED]";
	}
	return true;
}

void Trigger::setDebug(bool is_true)
{
	debug = is_true;
}

BasicSc2Bot* Trigger::getAgent() {
	return agent;
}

Precept::Precept(BasicSc2Bot* agent_) {
	agent = agent_;
	debug = false;
	has_directive = false;
	has_trigger = false;
}

Precept::~Precept() {
	directives.clear();
}


bool Precept::execute() {
	bool any_executed = false;
	for (auto d : directives) {
		if (d->execute(agent))
			any_executed = true;
	}
	return any_executed;
}

void Precept::addDirective(Directive directive_) {
	// once a directive has been added to a Precept, it cannot be modified
	// this ensures we can look up whether the same directive already exists for a unit

	if (debug) {
		directive_.setDebug(true);
	}
	directive_.lock();
	agent->storeDirective(directive_);
	Directive* dir_ = agent->getLastStoredDirective();
	directives.push_back(dir_);
	has_directive = true;
	//std::cout << "the pointer inside the provided directive at as it's added: " << directive_.strategy_ref << std::endl;
	//std::cout << "the pointer inside the stored directive at as it's added: " << dir_->strategy_ref << std::endl;
}

void Precept::addTrigger(Trigger trigger_) {
	if (debug) {
		trigger_.setDebug(true);
	}
	triggers.push_back(trigger_);
	has_trigger = true;
}

void Precept::setDebug(bool is_true)
{
	debug = is_true;
}

bool Precept::hasDirective() {
	return has_directive;
}

bool Precept::hasTrigger() {
	return has_trigger;
}

bool Precept::checkTriggerConditions() {
	for (Trigger t_ : triggers) {
		if (t_.checkConditions())
			return true;
		}
	}
	return false;
}