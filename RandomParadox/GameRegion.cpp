#include "GameRegion.h"



GameRegion::GameRegion()
{
}

GameRegion::GameRegion(const Region& baseRegion) : baseRegion(baseRegion), position(baseRegion.position), assigned(false)
{
	this->ID = baseRegion.ID;
	this->sea = baseRegion.sea;
}

GameRegion::~GameRegion()
{
}
