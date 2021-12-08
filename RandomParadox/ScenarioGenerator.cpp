#include "ScenarioGenerator.h"



ScenarioGenerator::ScenarioGenerator(FastWorldGenerator& f) : f(f)
{
}


ScenarioGenerator::~ScenarioGenerator()
{
}

void ScenarioGenerator::mapRegions()
{
	for (auto& region : f.provinceGenerator.regions)
	{
		GameRegion gR(region);
		for (auto& baseRegion : gR.baseRegion.neighbourRegions)
		{
			gR.neighbours.push_back(baseRegion);
		}
		gameRegions.push_back(gR);
	}
}

GameRegion& ScenarioGenerator::findStartRegion()
{
	vector<GameRegion> freeRegions;
	for (auto& gameRegion : gameRegions)
	{
		if (!gameRegion.assigned && !gameRegion.sea)
			freeRegions.push_back(gameRegion);
	}
	auto startRegion = *select_random(freeRegions);
	return gameRegions[startRegion.ID];
}

// generate countries according to given ruleset for each game
// TODO: rulesets, e.g. naming schemes? tags? country size?
void ScenarioGenerator::generateCountries()
{
	vector<std::string> tags = { "BRA", "GER", "FRA", "SOV", "USA", "ITA", "ENG", "GRE", "TUR", "CAN", "BUL" };
	for (auto tag : tags)
	{
		Country C(tag);
		auto startRegion(findStartRegion());
		C.assignRegions(10, gameRegions, startRegion);
		countryMap.emplace(tag, C);
	}


	for (auto& gameRegion : gameRegions)
	{
		if (!gameRegion.sea && !gameRegion.assigned)
		{
			auto x = getNearestAssignedLand(gameRegions, gameRegion, Data::getInstance().width, Data::getInstance().height);
			std::cout << x.owner << std::endl;
			countryMap.at(x.owner).addRegion(gameRegion);
		}
	}
}

void ScenarioGenerator::dumpDebugCountrymap(std::string path)
{
	Bitmap countryBMP(Data::getInstance().width, Data::getInstance().height, 24);
	for (auto& country : countryMap)
	{
		for (auto region : country.second.ownedRegions)
		{
			for (auto prov : region.baseRegion.provinces)
			{
				for (auto pix : prov->pixels)
				{
					countryBMP.setColourAtIndex(pix, country.second.colour);
				}
			}
		}
	}
	Bitmap::SaveBMPToFile(countryBMP, (path).c_str());
}


