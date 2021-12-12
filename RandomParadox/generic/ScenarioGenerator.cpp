#include "ScenarioGenerator.h"



ScenarioGenerator::ScenarioGenerator(FastWorldGenerator& f) : f(f)
{
	gamePaths["hoi4"] = "D:\\Steam\\steamapps\\common\\Hearts of Iron IV\\";
}


ScenarioGenerator::~ScenarioGenerator()
{
}



void ScenarioGenerator::loadRequiredResources(std::string gamePath)
{
	bitmaps["provinces"] = rLoader.loadProvinceMap(gamePath);
	bitmaps["heightmap"] = rLoader.loadHeightMap(gamePath);
	Data::getInstance().bufferBitmap("provinces", bitmaps["provinces"]);
}

void ScenarioGenerator::hoi4Preparations(bool useDefaultStates, bool useDefaultProvinces)
{
	bool loadDefaultRegions = false;
	
	loadRequiredResources(gamePaths["hoi4"]);
	Data::getInstance().width = bitmaps["provinces"].bInfoHeader.biWidth;
	Data::getInstance().height = bitmaps["provinces"].bInfoHeader.biHeight;
	Data::getInstance().bitmapSize = Data::getInstance().width*Data::getInstance().height;
	Data::getInstance().seaLevel = 94; //hardcoded for hoi4
	Data::getInstance().debugMapsPath = "debugMaps//";


	// get province map
	auto provinceMap = bitmaps["provinces"];
	auto heightMap = bitmaps["heightmap"].get24BitRepresentation();
	// write info to Data that is needed by FastWorldGenerator

	// now get info on provinces: who neighbours who, who is coastal...
	auto provinceDefinition = rLoader.loadDefinition(gamePaths["hoi4"]);
	provinceDefinition.erase(provinceDefinition.begin());
	set<int> tokensToConvert{ 0,1,2,3,7 };
	std::map<int, Province*> provinces;
	for (auto def : provinceDefinition)
	{
		auto numbers = ParserUtils::getNumbers(def, ';', tokensToConvert);
		if (!numbers.size() || numbers[0] == 0)
			continue;
		Colour colour{ (unsigned char)numbers[1],(unsigned char)numbers[2] ,(unsigned char)numbers[3] };
		Province * province = new Province();
		province->isLake = false;
		if (def.find("sea") != std::string::npos)
			province->sea = true;
		else if (def.find("lake") != std::string::npos)
		{
			province->isLake = true;
		}
		else
			province->sea = false;
		f.provinceGenerator.provinceMap.setValue(colour, province);
		f.provinceGenerator.provinceMap[colour]->provID = numbers[0] - 1;
		province->colour = colour;
		provinces[province->provID] = province;
		f.provinceGenerator.provinces.push_back(province);

	}
	f.provinceGenerator.provPixels(provinceMap);
	f.provinceGenerator.evaluateNeighbours(provinceMap);
	for (auto prov : f.provinceGenerator.provinces)
		prov->position.calcWeightedCenter(prov->pixels);

	if (loadDefaultRegions)
	{
		auto textRegions = rLoader.loadStates(gamePaths["hoi"]);
		for (auto textRegion : textRegions)
		{
			Region R;
			auto ID = ParserUtils::getLineValue(textRegion, "id", "=");
			R.ID = stoi(ID) - 1;
			std::cout << R.ID << std::endl;
			//R.provinces.push_back(f.provinceGenerator.provinceMap[colour]);
			auto stateProvinces = ParserUtils::getNumberBlock(textRegion, "provinces");
			for (auto stateProvince : stateProvinces)
			{
				stateProvince -= 1; // we count from 0, the game starts at 1
				provinces[stateProvince]->regionID = R.ID;
				if (provinces[stateProvince]->sea)
					R.sea = true;
				R.provinces.push_back(provinces[stateProvince]);
			}
			f.provinceGenerator.regions.push_back(R);
		}
	}

	else {
		// evaluate landmasses

		Bitmap terrainBMP = Bitmap(Data::getInstance().width, Data::getInstance().height, 24);
		//TerrainGenerator tG;
		//tG.createTerrain(terrainBMP, heightMap);
		//Bitmap::SaveBMPToFile(terrainBMP, (Data::getInstance().debugMapsPath + ("terrain.bmp")).c_str());
		//tG.detectContinents(terrainBMP);
		f.provinceGenerator.evaluateRegions(6);
	}

	Visualizer::prettyRegions(f.provinceGenerator);
	f.provinceGenerator.sortRegions();
	f.provinceGenerator.evaluateRegionNeighbours();
	Visualizer::provinceInfoMap3(provinceMap, f.provinceGenerator);
	Visualizer::provinceInfoMap4(provinceMap, f.provinceGenerator);

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
	if (freeRegions.size() == 0)
	{
		return gameRegions[0];
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
		if (startRegion.assigned || startRegion.sea)
			continue;
		C.assignRegions(6, gameRegions, startRegion);
		countryMap.emplace(tag, C);
	}
	for (auto& gameRegion : gameRegions)
	{
		if (!gameRegion.sea && !gameRegion.assigned)
		{
			auto x = getNearestAssignedLand(gameRegions, gameRegion, Data::getInstance().width, Data::getInstance().height);
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


