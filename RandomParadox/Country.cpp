#include "Country.h"

int Country::ID = 0;


Country::Country(std::string tag) : tag{tag}
{
	auto random = Data::getInstance().random2;
	colour = { random() % 255,random() % 255,random() % 255 };
}


Country::~Country()
{
}

void Country::assignRegions(int maxRegions, vector<GameRegion>& gameRegions, GameRegion& startRegion)
{
	addRegion(startRegion);
	auto breakCounter = 0;
	while (ownedRegions.size() < maxRegions && breakCounter++ < 10)
	{
		for (auto& gameRegion : ownedRegions)
		{
			if (ownedRegions.size() >= maxRegions)
				break;
			if (gameRegion.neighbours.size() == 0)
				break;
			auto nextRegion = *select_random(gameRegion.neighbours);
			if (!gameRegions[nextRegion].assigned && !gameRegions[nextRegion].sea)
			{
				addRegion(gameRegions[nextRegion]);
			}
		}
	}
}

void Country::addRegion(GameRegion& region)
{
	region.assigned = true;
	region.owner = tag;
	ownedRegions.push_back(region);
}
