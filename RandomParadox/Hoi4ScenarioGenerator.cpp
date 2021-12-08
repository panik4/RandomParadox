#include "Hoi4ScenarioGenerator.h"



Hoi4ScenarioGenerator::Hoi4ScenarioGenerator(FastWorldGenerator& f, ScenarioGenerator sG) : scenGen(sG)
{
	this->f = f;
	this->random = Data::getInstance().random2;
}


Hoi4ScenarioGenerator::~Hoi4ScenarioGenerator()
{
}




