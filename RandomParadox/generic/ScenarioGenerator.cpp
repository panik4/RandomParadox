#include "ScenarioGenerator.h"



ScenarioGenerator::ScenarioGenerator(FastWorldGenerator& f) : f(f)
{
	gamePaths["hoi4"] = "D:\\Steam\\steamapps\\common\\Hearts of Iron IV\\";
	Flag::readColourGroups();
	Flag::readFlagTypes();
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
	Data::getInstance().mapsPath = "Maps//";

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
		Bitmap::SaveBMPToFile(humidityBMP, (Data::getInstance().mapsPath + ("humidity.bmp")).c_str());
		climateGenerator.climateMap(climateMap, humidityBMP, heightMap, Data::getInstance().seaLevel);
		Bitmap::SaveBMPToFile(climateMap, (Data::getInstance().mapsPath + ("climate.bmp")).c_str());

		Bitmap::SaveBMPToFile(terrainBMP, (Data::getInstance().mapsPath + ("terrain.bmp")).c_str());
		Bitmap provinceMap(Data::getInstance().width, Data::getInstance().height, 24);
		f.provinceGenerator.generateProvinces(terrainBMP, provinceMap, riverMap, tG.landBodies);
		Bitmap::SaveBMPToFile(provinceMap, (Data::getInstance().mapsPath + ("provinces.bmp")).c_str());
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
	generateDevelopment();
	mapTerrain();
}

void ScenarioGenerator::mapContinents()
{
	std::cout << "Mapping Continents\n";
	auto ID = 0;
	for (auto& continent : f.provinceGenerator.continents)
	{
		GameContinent c;
		c.ID = continent.ID;
	}
}

void ScenarioGenerator::mapRegions()
{
	std::cout << "Mapping Regions\n";
	Visualizer::prettyRegions(f.provinceGenerator);
	for (auto& region : f.provinceGenerator.regions)
	{
		GameRegion gR(region);
		for (auto& baseRegion : gR.baseRegion.neighbourRegions)
		{
			gR.neighbours.push_back(baseRegion);
		}
		gR.name = nG.generateName();
		for (auto& province : gR.baseRegion.provinces)
		{
			GameProvince gP(province);
			for (auto& baseProvinceNeighbour : gP.baseProvince->adjProv)
			{
				gP.neighbours.push_back(baseProvinceNeighbour);
			}
			gP.name = nG.generateName();
			gR.gameProvinces.push_back(gP);
			gameProvinces.push_back(gP);
		}
		gameRegions.push_back(gR);
	}
	std::sort(gameProvinces.begin(), gameProvinces.end());
}

void ScenarioGenerator::generatePopulations()
{
	std::cout << "Generating Population\n";
	auto popMap = Data::getInstance().findBitmapByKey("population");
	auto cityMap = Data::getInstance().findBitmapByKey("cities");
	for (auto& c : countryMap)
		for (auto& gameRegion : c.second.ownedRegions)
			for (auto& gameProv : gameRegion.gameProvinces)
			{
				gameProv.popFactor = 0.1 + popMap.getColourAtIndex(gameProv.baseProvince->position.weightedCenter) / Data::getInstance().namedColours["population"];
				int cityPixels = 0;
				for (auto pix : gameProv.baseProvince->pixels)
				{
					if (cityMap.getColourAtIndex(pix) == Data::getInstance().namedColours["cities"])
					{
						cityPixels++;
					}
				}
				gameProv.cityShare = (double)cityPixels / gameProv.baseProvince->pixels.size();
			}
}

void ScenarioGenerator::generateDevelopment()
{
	// high pop-> high development
	// high city share->high dev
	// terrain type?
	// .....
	std::cout << "Generating State Development\n";
	auto cityBMP = Data::getInstance().findBitmapByKey("cities");
	for (auto& c : countryMap)
		for (auto& gameProv : c.second.ownedRegions)
			for (auto& gameProv : gameProv.gameProvinces)
			{
				auto cityDensity = 0;
				if (gameProv.baseProvince->cityPixels.size())
				{
					cityDensity = cityBMP.getColourAtIndex(gameProv.baseProvince->cityPixels[0]) / Data::getInstance().namedColours["cities"];
				}
				gameProv.devFactor = clamp(0.2 + 0.5 * gameProv.popFactor + 1.0 * gameProv.cityShare * cityDensity, 0.0, 1.0);
			}
}

void ScenarioGenerator::mapTerrain()
{
	auto namedColours = Data::getInstance().namedColours;
	auto climateMap = Data::getInstance().findBitmapByKey("climate");
	Bitmap typeMap(climateMap.bInfoHeader.biWidth, climateMap.bInfoHeader.biHeight, 24);
	std::cout << "Mapping Terrain\n";
	vector<std::string> targetTypes{ "plains", "forest", "marsh", "hills", "mountain", "desert", "urban", "jungle" };
	
	for (auto& c : countryMap)
		for (auto& gameRegion : c.second.ownedRegions)
			for (auto& gameProv : gameRegion.gameProvinces)
			{
				std::map<Colour, int> colourPrevalence;
				for (auto& pix : gameProv.baseProvince->pixels)
				{
					if (colourPrevalence[climateMap.getColourAtIndex(pix)])
						colourPrevalence[climateMap.getColourAtIndex(pix)]++;
					else
						colourPrevalence[climateMap.getColourAtIndex(pix)] = 1;
				}

				using pair_type = decltype(colourPrevalence)::value_type;
				auto pr = std::max_element
				(
					std::begin(colourPrevalence), std::end(colourPrevalence),
					[](const pair_type & p1, const pair_type & p2) {
					return p1.second < p2.second;
				}
				);
				if (pr->first == namedColours["jungle"])
					gameProv.terrainType = "jungle";
				else if (pr->first == namedColours["forest"])
					gameProv.terrainType = "forest";
				else if (pr->first == namedColours["lowMountains"])
					gameProv.terrainType = "hills";
				else if (pr->first == namedColours["mountains"] || pr->first == namedColours["peaks"])
					gameProv.terrainType = "mountain";
				else if (pr->first == namedColours["grassland"] || pr->first == namedColours["savannah"])
					gameProv.terrainType = "plains";
				else if (pr->first == namedColours["desert"])
					gameProv.terrainType = "desert";
				else
					gameProv.terrainType = "plains";
				gameProvinces[gameProv.ID].terrainType = gameProv.terrainType;
				for (auto pix : gameProv.baseProvince->pixels)
				{
					if (pr->first == namedColours["jungle"])
						typeMap.setColourAtIndex(pix, Colour{ 255,255,0 });
					else if (pr->first == namedColours["forest"])
						typeMap.setColourAtIndex(pix, Colour{ 0,255,0 });
					else if (pr->first == namedColours["lowMountains"])
						typeMap.setColourAtIndex(pix, Colour{ 128,128,128 });
					else if (pr->first == namedColours["mountains"] || pr->first == namedColours["peaks"])
						typeMap.setColourAtIndex(pix, Colour{ 255,255,255 });
					else if (pr->first == namedColours["grassland"] || pr->first == namedColours["savannah"])
						typeMap.setColourAtIndex(pix, Colour{ 0,255,128 });
					else if (pr->first == namedColours["desert"])
						typeMap.setColourAtIndex(pix, Colour{ 0,255,255 });
					else
						typeMap.setColourAtIndex(pix, Colour{ 255,0,0 });
				}
			}
	Bitmap::SaveBMPToFile(typeMap, "debugMaps/typeMap.bmp");
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
	std::cout << "Generating Countries\n";
	auto forbiddenTags = rLoader.loadForbiddenTags(gamePaths["hoi4"]);
	for (auto tag : forbiddenTags)
	{
		tags.insert(tag);
	}
	for (int i = 0; i < 30; i++)
	{
		// Get Name
		auto name = nG.generateName();
		// Tag from Name
		auto tag = nG.generateTag(name, tags);
		// generate flag
		Country C(tag);
		C.name = name;
		C.adjective = nG.generateAdjective(name);
		Flag f(Data::getInstance().random2, 82, 52);
		C.flag = f;
		C.ID = i;
		C.developmentFactor = Data::getInstance().getRandomDouble(0.1, 1.0);
		countryMap.emplace(tag, C);
	}
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
			countryMap.at(x.owner).addRegion(gameRegion, gameRegions);
		}
	}
}

void ScenarioGenerator::evaluateNeighbours()
{
	std::cout << "Evaluating Country Neighbours\n";
	for (auto& c : countryMap)
	{
		for (auto& gameRegion : c.second.ownedRegions)
		{
			for (auto& neighbourRegion : gameRegion.neighbours)
			{
				if (gameRegions[neighbourRegion].owner != c.first)
					c.second.neighbours.insert(gameRegions[neighbourRegion].owner);
			}
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
	Data::getInstance().bufferBitmap("countries", countryBMP);
	Bitmap::SaveBMPToFile(countryBMP, (path).c_str());
}


