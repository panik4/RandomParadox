#include "Country.h"

int Country::ID = 0;


Country::Country(std::string tag) : tag{ tag }
{
	auto random = Data::getInstance().random2;
	colour = { random() % 255,random() % 255,random() % 255 };
}


Country::~Country()
{
}

void Country::assignRegions(int maxRegions, vector<GameRegion>& gameRegions, GameRegion& startRegion)
{
	addRegion(startRegion, gameRegions);
	auto breakCounter = 0;
	while (ownedRegions.size() < maxRegions && breakCounter++ < 100)
	{
		for (auto& gameRegion : ownedRegions)
		{
			if (ownedRegions.size() >= maxRegions)
				break;
			if (gameRegion.neighbours.size() == 0)
				continue;
			auto nextRegion = *select_random(gameRegion.neighbours);
			if (!gameRegions[nextRegion].assigned && !gameRegions[nextRegion].sea)
			{
				gameRegions[nextRegion].assigned = true;
				addRegion(gameRegions[nextRegion], gameRegions);
			}
		}
	}
}

void Country::addRegion(GameRegion& region, vector<GameRegion>& gameRegions)
{
	gameRegions[region.ID].assigned = true;
	gameRegions[region.ID].owner = tag;
	region.assigned = true;
	region.owner = tag;
	std::cout << region.ID << std::endl;
	ownedRegions.push_back(region);
}
