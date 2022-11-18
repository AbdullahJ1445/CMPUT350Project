#pragma once
#include "sc2api/sc2_api.h"
//#include "Agents.h"
#include "Mob.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_typeenums.h"

class Mob;
class BotAgent;

class MobHandler {
public:
    MobHandler(BotAgent* agent);
    void set_mob_idle(Mob* mob_, bool is_true=true);
    void set_mob_busy(Mob* mob_, bool is_true=true);
    bool addMob(Mob mob_);
    bool mob_exists(const sc2::Unit& unit);
    Mob& getMob(const sc2::Unit& unit);
    std::unordered_set<Mob*> getIdleWorkers();
    std::unordered_set<Mob*> filter_by_flag(std::unordered_set<Mob*> mobs_set, FLAGS flag, bool is_true=true);
    std::unordered_set<Mob*> filter_by_flags(std::unordered_set<Mob*> mobs_set, std::unordered_set<FLAGS> flag_list, bool is_true=true);
    std::unordered_set<Mob*> get_mobs();
    std::unordered_set<Mob*> get_idle_mobs();
    std::unordered_set<Mob*> get_busy_mobs();
    
private:
    BotAgent* agent;
    //data containers
    std::vector<std::shared_ptr<Mob>> mobs_storage; 
	std::unordered_set<Mob*> mobs; 
	std::unordered_set<Mob*> idle_mobs;
    std::unordered_set<Mob*> busy_mobs;
	std::unordered_map<sc2::Tag, Mob*> mob_by_tag; 

};





