#pragma once
#include "CurrentStrategy.h"

class BotAgent;
class StrategyOrder;
class Trigger;
class Directive;


struct CurrentStrategy {
	CurrentStrategy(BotAgent* bot_);

	void loadStrategies();
	private:
		BotAgent* bot;
};