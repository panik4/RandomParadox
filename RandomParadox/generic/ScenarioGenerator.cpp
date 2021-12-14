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
	loadRequiredResources(gamePaths["hoi4"]);
	auto heightMap = bitmaps["heightmap"].get24BitRepresentation();
	Data::getInstance().bufferBitmap("heightmap", heightMap);
	Data::getInstance().width = bitmaps["heightmap"].bInfoHeader.biWidth;
	Data::getInstance().height = bitmaps["heightmap"].bInfoHeader.biHeight;
	Data::getInstance().bitmapSize = Data::getInstance().width*Data::getInstance().height;
	Data::getInstance().seaLevel = 94; //hardcoded for hoi4
	Data::getInstance().debugMapsPath = "debugMaps//";

	Bitmap terrainBMP = Bitmap(Data::getInstance().width, Data::getInstance().height, 24);
	TerrainGenerator tG;

	std::map<int, Province*> provinces;
	if (useDefaultProvinces)
	{
		// get province map
		auto provinceMap = bitmaps["provinces"];
		// write info to Data that is needed by FastWorldGenerator

		// now get info on provinces: who neighbours who, who is coastal...
		auto provinceDefinition = rLoader.loadDefinition(gamePaths["hoi4"]);
		provinceDefinition.erase(provinceDefinition.begin());
		set<int> tokensToConvert{ 0,1,2,3,7 };
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
	}
	else {
		Data::getInstance().calcParameters();
		Bitmap riverMap(Data::getInstance().width, Data::getInstance().height, 24);
		Bitmap humidityBMP(Data::getInstance().width, Data::getInstance().height, 24);
		Bitmap climateMap(Data::getInstance().width, Data::getInstance().height, 24);
		tG.createTerrain(terrainBMP, heightMap);
		ClimateGenerator climateGenerator;
		climateGenerator.setProvinceGenerator(&f.provinceGenerator);
		climateGenerator.humidityMap(heightMap, humidityBMP, riverMap, tG, Data::getInstance().seaLevel, Data::getInstance().updateThreshold);
		Bitmap::SaveBMPToFile(humidityBMP, (Data::getInstance().debugMapsPath + ("humidity.bmp")).c_str());
		climateGenerator.climateMap(climateMap, humidityBMP, heightMap, Data::getInstance().seaLevel, Data::getInstance().updateThreshold);
		Bitmap::SaveBMPToFile(climateMap, (Data::getInstance().debugMapsPath + ("climate.bmp")).c_str());

		Bitmap::SaveBMPToFile(terrainBMP, (Data::getInstance().debugMapsPath + ("terrain.bmp")).c_str());
		Bitmap provinceMap(Data::getInstance().width, Data::getInstance().height, 24);
		f.provinceGenerator.generateProvinces(terrainBMP, provinceMap, riverMap, 100);
		Bitmap::SaveBMPToFile(provinceMap, (Data::getInstance().debugMapsPath + ("provinces.bmp")).c_str());
		bitmaps["provinces"] = provinceMap;
		f.provinceGenerator.createProvinceMap();
		f.provinceGenerator.beautifyProvinces(provinceMap, riverMap);
		f.provinceGenerator.evaluateNeighbours(provinceMap);
		tG.detectContinents(terrainBMP);
		f.provinceGenerator.evaluateRegions(3);
		f.provinceGenerator.evaluateContinents(10, Data::getInstance().width, Data::getInstance().height, tG);
		//genericParser.writeAdjacency((Data::getInstance().debugMapsPath + ("adjacency.csv")).c_str(), provinceGenerator.provinces);		
		//genericParser.writeDefinition((Data::getInstance().debugMapsPath + ("definition.csv")).c_str(), provinceGenerator.provinces);
		Visualizer::provinceInfoMap(provinceMap, f.provinceGenerator);
		Visualizer::provinceInfoMap2(provinceMap, f.provinceGenerator);
		Visualizer::provinceInfoMap3(provinceMap, f.provinceGenerator);
	}
	if (useDefaultStates)
	{
		auto textRegions = rLoader.loadStates(gamePaths["hoi4"]);
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

		//tG.detectContinents(terrainBMP);
		f.provinceGenerator.evaluateRegions(3);
	}

	auto provinceMap = bitmaps["provinces"];
	f.provinceGenerator.sortRegions();
	f.provinceGenerator.evaluateRegionNeighbours();
	Visualizer::provinceInfoMap3(provinceMap, f.provinceGenerator);
	Visualizer::provinceInfoMap4(provinceMap, f.provinceGenerator);

}

void ScenarioGenerator::generateWorld()
{
	generatePopulations();
}

void ScenarioGenerator::mapRegions()
{
	Visualizer::prettyRegions(f.provinceGenerator);
	for (auto& region : f.provinceGenerator.regions)
	{
		GameRegion gR(region);
		for (auto& baseRegion : gR.baseRegion.neighbourRegions)
		{
			gR.neighbours.push_back(baseRegion);
		}
		gR.name = nG.generateName();
		gameRegions.push_back(gR);
	}
}

void ScenarioGenerator::generatePopulations()
{

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
	for (int i = 0; i < 100; i++)
	{
		// Get Name
		auto name = nG.generateName();
		// Tag from Name
		auto tag = nG.generateTag(name, tags);
		//Flag f();
		// generate flag
		Country C(tag);
		Flag f(Data::getInstance().random2, 82, 52);
		C.flag = f;
		countryMap.emplace(tag, C);

	}
	//vector<std::string> tags = { "BRA", "GER", "FRA", "SOV", "USA", "ITA", "ENG", "GRE", "TUR", "CAN", "BUL" };
	//for (auto tag : tags)
	//{
	//	Country C(tag);

	//	countryMap.emplace(tag, C);
	//}
	for (auto& c : countryMap)
	{
		auto startRegion(findStartRegion());
		if (startRegion.assigned || startRegion.sea)
			continue;
		c.second.assignRegions(6, gameRegions, startRegion);
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


