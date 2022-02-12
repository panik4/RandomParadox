#include "Country.h"

Country::Country()
{}

Country::Country(std::string tag) : tag{ tag }
{
	auto random = Data::getInstance().random2;
	colour = { random() % 255,random() % 255,random() % 255 };
}


Country::~Country()
{}

void Country::assignRegions(int maxRegions, vector<GameRegion>& gameRegions, GameRegion& startRegion, vector<GameProvince>& gameProvinces)
{
	addRegion(startRegion, gameRegions, gameProvinces);
	auto breakCounter = 0;
	while (ownedRegions.size() < maxRegions && breakCounter++ < 100) {
		for (auto& gameRegion : ownedRegions) {
			if (ownedRegions.size() >= maxRegions)
				break;
			if (gameRegion.neighbours.size() == 0)
				continue;
			auto nextRegion = *select_random(gameRegion.neighbours);
			if (!gameRegions[nextRegion].assigned && !gameRegions[nextRegion].sea) {
				gameRegions[nextRegion].assigned = true;
				addRegion(gameRegions[nextRegion], gameRegions, gameProvinces);
			}
		}
	}
}

void Country::addRegion(GameRegion& region, vector<GameRegion>& gameRegions, vector<GameProvince>& gameProvinces)
{
	gameRegions[region.ID].assigned = true;
	gameRegions[region.ID].owner = tag;
	for (auto& gameProvince : gameRegions[region.ID].gameProvinces)
		gameProvince.owner = tag;
	region.assigned = true;
	region.owner = tag;
	for (auto& gameProvince : region.gameProvinces)
		gameProvince.owner = tag;
	ownedRegions.push_back(region);
}
