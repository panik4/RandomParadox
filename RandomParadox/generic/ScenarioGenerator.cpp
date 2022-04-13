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
	Bitmap::bufferBitmap("provinces", bitmaps["provinces"]);
}

void ScenarioGenerator::hoi4Preparations(bool useDefaultStates, bool useDefaultProvinces)
{
	loadRequiredResources(gamePaths["hoi4"]);
	auto heightMap = bitmaps["heightmap"].get24BitRepresentation();
	Bitmap::bufferBitmap("heightmap", heightMap);
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
		const std::set<int> tokensToConvert{ 0,1,2,3,7 };
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
			f.provinceGenerator.provinceMap[colour]->ID = numbers[0] - 1;
			province->colour = colour;
			provinces[province->ID] = province;
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
		climateGenerator.humidityMap(f.provinceGenerator.provinces, heightMap, humidityBMP, riverMap, Data::getInstance().seaLevel);
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
		f.provinceGenerator.generateRegions(3);
		f.provinceGenerator.evaluateContinents(Data::getInstance().width, Data::getInstance().height, tG.continents, tG.landBodies);
		//genericParser.writeAdjacency((Data::getInstance().debugMapsPath + ("adjacency.csv")).c_str(), provinceGenerator.provinces);		
		//genericParser.writeDefinition((Data::getInstance().debugMapsPath + ("definition.csv")).c_str(), provinceGenerator.provinces);
		Visualizer::provinceInfoMapNeighbours(provinceMap, f.provinceGenerator.provinces);
		Visualizer::provinceInfoMapCoasts(provinceMap, f.provinceGenerator.provinces);
		Visualizer::provinceInfoMapBorders(provinceMap, f.provinceGenerator.provinces);
	}
	if (useDefaultStates) {
		auto textRegions = rLoader.loadStates(gamePaths["hoi4"]);
		for (auto textRegion : textRegions) {
			Region R;
			auto ID = ParserUtils::getLineValue(textRegion, "id", "=");
			R.ID = stoi(ID) - 1;
			//R.provinces.push_back(f.provinceGenerator.provinceMap[colour]);
			auto stateProvinces = ParserUtils::getNumberBlock(textRegion, "provinces");
			for (auto stateProvince : stateProvinces) {
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
		f.provinceGenerator.generateRegions(3);
	}

	auto provinceMap = bitmaps["provinces"];
	f.provinceGenerator.sortRegions();
	f.provinceGenerator.evaluateRegionNeighbours();
	Visualizer::provinceInfoMapBorders(provinceMap, f.provinceGenerator.provinces);
	Visualizer::provinceInfoMapClassification(provinceMap, f.provinceGenerator.provinces);

}

void ScenarioGenerator::generateWorld()
{
	generatePopulations();
	generateDevelopment();
	mapTerrain();
}

void ScenarioGenerator::mapContinents()
{
	UtilLib::logLine("Mapping Continents");
	auto ID = 0;
	for (const auto& continent : f.provinceGenerator.continents) {
		GameContinent c;
		c.ID = continent.ID;
	}
}

void ScenarioGenerator::mapRegions()
{
	UtilLib::logLine("Mapping Regions");
	for (auto& region : f.provinceGenerator.regions) {
		GameRegion gR(region);
		for (auto& baseRegion : gR.baseRegion.neighbours)
			gR.neighbours.push_back(baseRegion);
		// generate random name for region
		gR.name = nG.generateName();
		// now create gameprovinces from FastWorldGen provinces
		for (auto& province : gR.baseRegion.provinces) {
			GameProvince gP(province);
			// also copy neighbours
			for (auto& baseProvinceNeighbour : gP.baseProvince->neighbours)
				gP.neighbours.push_back(baseProvinceNeighbour);
			// give name to province
			gP.name = nG.generateName();
			gR.gameProvinces.push_back(gP);
			gameProvinces.push_back(gP);
		}
		// save game region
		gameRegions.push_back(gR);
	}
	// check if we have the same amount of gameProvinces as FastWorldGen provinces
	if (gameProvinces.size() != f.provinceGenerator.provinces.size())
		UtilLib::logLine("Fatal: Lost provinces");
	// sort by gameprovince ID
	std::sort(gameProvinces.begin(), gameProvinces.end());
}

void ScenarioGenerator::generatePopulations()
{
	UtilLib::logLine("Generating Population");
	auto popMap = Bitmap::findBitmapByKey("population");
	auto cityMap = Bitmap::findBitmapByKey("cities");
	for (auto& c : countryMap)
		for (auto& gameRegion : c.second.ownedRegions)
			for (auto& gameProv : gameRegion.gameProvinces) {
				// calculate the population factor
				gameProv.popFactor = 0.1 + popMap.getColourAtIndex(gameProv.baseProvince->position.weightedCenter) / Data::getInstance().namedColours["population"];
				int cityPixels = 0;
				// calculate share of province that is a city
				for (auto pix : gameProv.baseProvince->pixels)
					if (cityMap.getColourAtIndex(pix).isShadeOf(Data::getInstance().namedColours["cities"]))
						cityPixels++;
				gameProv.cityShare = (double)cityPixels / gameProv.baseProvince->pixels.size();
			}
}

void ScenarioGenerator::generateDevelopment()
{
	// high pop-> high development
	// high city share->high dev
	// terrain type?
	// .....
	UtilLib::logLine("Generating State Development");
	auto cityBMP = Bitmap::findBitmapByKey("cities");
	for (auto& c : countryMap)
		for (auto& gameProv : c.second.ownedRegions)
			for (auto& gameProv : gameProv.gameProvinces) {
				auto cityDensity = 0.0;
				// calculate development with density of city and population factor
				if (gameProv.baseProvince->cityPixels.size())
					cityDensity = cityBMP.getColourAtIndex(gameProv.baseProvince->cityPixels[0]) / Data::getInstance().namedColours["cities"];
				gameProv.devFactor = UtilLib::clamp(0.2 + 0.5 * gameProv.popFactor + 1.0 * gameProv.cityShare * cityDensity, 0.0, 1.0);
			}
}

void ScenarioGenerator::mapTerrain()
{
	auto namedColours = Data::getInstance().namedColours;
	auto climateMap = Bitmap::findBitmapByKey("climate");
	Bitmap typeMap(climateMap.bInfoHeader.biWidth, climateMap.bInfoHeader.biHeight, 24);
	UtilLib::logLine("Mapping Terrain");
	std::vector<std::string> targetTypes{ "plains", "forest", "marsh", "hills", "mountain", "desert", "urban", "jungle" };

	for (auto& c : countryMap)
		for (auto& gameRegion : c.second.ownedRegions)
			for (auto& gameProv : gameRegion.gameProvinces) {
				std::map<Colour, int> colourPrevalence;
				for (auto& pix : gameProv.baseProvince->pixels) {
					if (colourPrevalence[climateMap.getColourAtIndex(pix)])
						colourPrevalence[climateMap.getColourAtIndex(pix)]++;
					else
						colourPrevalence[climateMap.getColourAtIndex(pix)] = 1;
				}
				// find the most prevalent colour in province, which sets the terrain type
				using pair_type = decltype(colourPrevalence)::value_type;
				auto pr = std::max_element(
					std::begin(colourPrevalence), std::end(colourPrevalence),
					[](const pair_type & p1, const pair_type & p2) {
					return p1.second < p2.second;
				}
				);
				// now check which it is and set the terrain type
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
				for (auto pix : gameProv.baseProvince->pixels) {
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
	Bitmap::SaveBMPToFile(typeMap, "Maps/typeMap.bmp");
}

GameRegion& ScenarioGenerator::findStartRegion()
{
	std::vector<GameRegion> freeRegions;
	for (const auto& gameRegion : gameRegions)
		if (!gameRegion.assigned && !gameRegion.sea)
			freeRegions.push_back(gameRegion);

	if (freeRegions.size() == 0)
		return gameRegions[0];

	const auto startRegion = *UtilLib::select_random(freeRegions);
	return gameRegions[startRegion.ID];
}

// generate countries according to given ruleset for each game
// TODO: rulesets, e.g. naming schemes? tags? country size?
void ScenarioGenerator::generateCountries(int numCountries)
{
	this->numCountries = numCountries;
	UtilLib::logLine("Generating Countries");
	// load tags from hoi4 that are used by the base game
	// do not use those to avoid conflicts
	const auto forbiddenTags = rLoader.loadForbiddenTags(gamePaths["hoi4"]);
	for (const auto& tag : forbiddenTags)
		tags.insert(tag);

	for (int i = 0; i < numCountries; i++) {
		// Get Name
		auto name = nG.generateName();
		// Tag from Name
		auto tag = nG.generateTag(name, tags);
		// generate flag
		Country C(tag, i);
		// get name and andjective
		C.name = name;
		C.adjective = nG.generateAdjective(name);
		// get a flag
		Flag f(Data::getInstance().random2, 82, 52);
		C.flag = f;
		// randomly set development of countries
		C.developmentFactor = Data::getInstance().getRandomDouble(0.1, 1.0);
		countryMap.emplace(tag, C);
	}
	for (auto& c : countryMap) {
		auto startRegion(findStartRegion());
		if (startRegion.assigned || startRegion.sea)
			continue;
		c.second.assignRegions(6, gameRegions, startRegion, gameProvinces);
	}
	for (auto& gameRegion : gameRegions) {
		if (!gameRegion.sea && !gameRegion.assigned) {
			auto x = UtilLib::getNearestAssignedLand(gameRegions, gameRegion, Data::getInstance().width, Data::getInstance().height);
			countryMap.at(x.owner).addRegion(gameRegion, gameRegions, gameProvinces);
		}
	}
}

void ScenarioGenerator::evaluateNeighbours()
{
	UtilLib::logLine("Evaluating Country Neighbours");
	for (auto& c : countryMap)
		for (const auto& gameRegion : c.second.ownedRegions)
			for (const auto& neighbourRegion : gameRegion.neighbours)
				if (gameRegions[neighbourRegion].owner != c.first)
					c.second.neighbours.insert(gameRegions[neighbourRegion].owner);
}

void ScenarioGenerator::dumpDebugCountrymap(std::string path)
{
	UtilLib::logLine("Mapping Continents");
	Bitmap countryBMP(Data::getInstance().width, Data::getInstance().height, 24);
	for (const auto& country : countryMap)
		for (const auto& region : country.second.ownedRegions)
			for (const auto& prov : region.baseRegion.provinces)
				for (const auto& pix : prov->pixels)
					countryBMP.setColourAtIndex(pix, country.second.colour);

	Bitmap::bufferBitmap("countries", countryBMP);
	Bitmap::SaveBMPToFile(countryBMP, (path).c_str());
}


