#include "GameProvince.h"



GameProvince::GameProvince(Province* province) : baseProvince(province), ID(province->ID)
{
}

GameProvince::~GameProvince()
{
}
