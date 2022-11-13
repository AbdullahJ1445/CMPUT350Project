#pragma once
#include "CurrentStrategy.h"

class BotAgent;
class StrategyOrder;
class Trigger;
class Directive;


CurrentStrategy::CurrentStrategy(BotAgent* bot_) {
	bot = bot_;
}

void CurrentStrategy::loadStrategies() {
	// How to add a new StrategyOrder to the bot's portfolio:
	// Create a StrategyOrder, Create a Directive, set the StrategyOrder directive, add TriggerCondition(s), push_back into strategies vector

}
