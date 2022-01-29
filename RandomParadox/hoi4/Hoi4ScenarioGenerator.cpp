#include "Hoi4ScenarioGenerator.h"


Hoi4ScenarioGenerator::Hoi4ScenarioGenerator()
{
	this->random = Data::getInstance().random2;
}


Hoi4ScenarioGenerator::~Hoi4ScenarioGenerator()
{
}

void Hoi4ScenarioGenerator::generateStateResources(ScenarioGenerator& scenGen)
{
	for (auto& c : scenGen.countryMap) {
		for (auto& gameRegion : c.second.ownedRegions) {
			for (auto& resource : resources) {
				auto chance = resource.second[2];
				if (random() % 100 < chance * 100.0) {
					// calc total of this resource
					auto totalOfResource = resource.second[1] * resource.second[0];
					// more per selected state if the chance is lower
					double averagePerState = (totalOfResource / (double)landStates) * (1.0 / chance);
					// range 1 to (2 times average - 1)
					double value = 1 + random() % (int)((2.0 * averagePerState));
					// increase by industry factor
					value *= industryFactor;
					gameRegion.attributeDoubles[resource.first] = value;
					// track amount of deployed resources
					if (resource.first == "aluminium")
						totalAluminium += value;
					else if (resource.first == "chromium")
						totalChromium += value;
					else if (resource.first == "rubber")
						totalRubber += value;
					else if (resource.first == "oil")
						totalOil += value;
					else if (resource.first == "steel")
						totalSteel += value;
					else if (resource.first == "tungsten")
						totalTungsten += value;
				}
			}
		}
	}
}

void Hoi4ScenarioGenerator::generateStateSpecifics(ScenarioGenerator& scenGen)
{
	// calculate the world land area
	double worldArea = (double)(Data::getInstance().bitmapSize / 3) * Data::getInstance().landMassPercentage;
	// calculate the target industry amount
	auto targetWorldIndustry = (double)Data::getInstance().landMassPercentage * 3648.0 * (sqrt(Data::getInstance().bitmapSize) / sqrt((double)(5632 * 2048)));
	for (auto& c : scenGen.countryMap) {
		for (auto& gameRegion : c.second.ownedRegions) {
			// count the number of land states for resource generation
			landStates++;
			int totalPop = 0;
			double totalStateArea = 0;
			double totalDevFactor = 0;
			double totalPopFactor = 0;
			for (const auto& gameProv : gameRegion.gameProvinces) {
				totalDevFactor += gameProv.devFactor / (double)gameRegion.gameProvinces.size();
				totalPopFactor += gameProv.popFactor / (double)gameRegion.gameProvinces.size();
				totalStateArea += gameProv.baseProvince->pixels.size();
			}
			gameRegion.attributeDoubles["stateCategory"] = clamp((int)(totalPopFactor * 5.0 + totalDevFactor * 6.0), 0, 9);
			if (gameRegion.gameProvinces.size() == 1) {
				gameRegion.attributeDoubles["stateCategory"] = 1;
			}
			gameRegion.attributeDoubles["development"] = totalDevFactor;
			gameRegion.attributeDoubles["population"] = totalStateArea * 1250.0 * totalPopFactor * worldPopulationFactor;
			worldPop += gameRegion.attributeDoubles["population"];
			// count the total coastal provinces of this region
			auto totalCoastal = 0;
			for (auto& gameProv : gameRegion.gameProvinces) {
				if (gameProv.baseProvince->coastal) {
					totalCoastal++;
					// only create a naval base, if a coastal supply hub was determined in this province
					if (gameProv.attributeDoubles["naval_bases"] == 1)
						gameProv.attributeDoubles["naval_bases"] = Data::getInstance().getRandomNumber(1, 5);
				}
				else {
					gameProv.attributeDoubles["naval_bases"] = 0;
				}
			}
			auto stateIndustry = (totalStateArea / worldArea) * totalPopFactor * targetWorldIndustry;
			if (totalCoastal > 0) {
				gameRegion.attributeDoubles["dockyards"] = clamp((int)round(stateIndustry * (0.25)), 0, 4);
				gameRegion.attributeDoubles["civilianFactories"] = clamp((int)round(stateIndustry * (0.5)), 0, 8);
				gameRegion.attributeDoubles["armsFactories"] = clamp((int)round(stateIndustry * (0.25)), 0, 4);
				militaryIndustry += gameRegion.attributeDoubles["armsFactories"];
				civilianIndustry += gameRegion.attributeDoubles["civilianFactories"];
				navalIndustry += gameRegion.attributeDoubles["dockyards"];
			}
			else {
				gameRegion.attributeDoubles["civilianFactories"] = clamp((int)round(stateIndustry * (0.6)), 0, 8);
				gameRegion.attributeDoubles["armsFactories"] = clamp((int)round(stateIndustry * (0.4)), 0, 4);
				gameRegion.attributeDoubles["dockyards"] = 0;
				militaryIndustry += gameRegion.attributeDoubles["armsFactories"];
				civilianIndustry += gameRegion.attributeDoubles["civilianFactories"];
			}
		}
	}
}

void Hoi4ScenarioGenerator::generateCountrySpecifics(ScenarioGenerator& scenGen, std::map<std::string, Country>& countries)
{
	std::cout << "HOI4: Generating Country Specifics\n";
	// graphical culture pairs:
	// { graphical_culture = type }
	// { graphical_culture_2d = type_2d }
	// {western_european_gfx, western_european_2d}
	// {eastern_european_gfx, eastern_european_2d}
	// {middle_eastern_gfx, middle_eastern_2d}
	// {african_gfx, african_2d}
	// {southamerican_gfx, southamerican_2d}
	// {commonwealth_gfx, commonwealth_2d}
	// {asian_gfx, asian_2d}
	vector<std::string> gfxCultures{ "western_european", "eastern_european", "middle_eastern", "african", "southamerican", "commonwealth", "asian" };
	vector<std::string> ideologies{ "fascism", "democratic", "communism", "neutrality" };
	for (auto& c : countries) {
		// select a random country ideology
		c.second.attributeStrings["gfxCulture"] = *select_random(gfxCultures);
		vector<double> popularities{};
		double totalPop = 0;
		for (int i = 0; i < 4; i++) {
			popularities.push_back(Data::getInstance().getRandomNumber(1, 100));
			totalPop += popularities[i];
		}
		auto sumPop = 0;
		for (int i = 0; i < 4; i++) {
			popularities[i] = popularities[i] / totalPop * 100;
			sumPop += popularities[i];
			int offset = 0;
			// to ensure a total of 100 as the sum for all ideologies
			if (i == 3 && sumPop < 100) {
				offset = 100 - sumPop;
			}
			c.second.attributeDoubles[ideologies[i]] = popularities[i] + offset;
		}
		// assign a ruling party
		c.second.attributeStrings["rulingParty"] = ideologies[Data::getInstance().getRandomNumber(0, ideologies.size())];
		// allow or forbid elections
		if (c.second.attributeStrings["rulingParty"] == "democratic")
			c.second.attributeDoubles["allowElections"] = 1;
		else if (c.second.attributeStrings["rulingParty"] == "neutrality")
			c.second.attributeDoubles["allowElections"] = Data::getInstance().getRandomNumber(0, 1);
		else
			c.second.attributeDoubles["allowElections"] = 0;
		// now get the full name of the country
		c.second.attributeStrings["fullName"] = scenGen.nG.modifyWithIdeology(c.second.attributeStrings["rulingParty"], c.second.name, c.second.adjective);
	}

}

void Hoi4ScenarioGenerator::generateLogistics(ScenarioGenerator& scenGen)
{
	std::cout << "Building rail networks\n";
	auto width = Data::getInstance().width;
	Bitmap logistics = Data::getInstance().findBitmapByKey("countries");
	for (auto& c : scenGen.countryMap) {
		// GameProvince ID, distance
		std::map<double, int> supplyHubs;
		// add capital
		auto capitalPosition = scenGen.gameRegions[c.second.capitalRegionID].position;
		auto capitalProvince = select_random(scenGen.gameRegions[c.second.capitalRegionID].gameProvinces);
		vector<double> distances;
		// region ID, provinceID
		std::map<int, GameProvince> supplyHubProvinces;
		std::map<int, bool> navalBases;
		std::set<int> gProvIDs;
		bool connectedNavalBase = false;
		for (auto& region : c.second.ownedRegions) {
			if (region.attributeDoubles["stateCategory"] > 6 && region.ID != c.second.capitalRegionID
				// if we're nearing the end of our region vector, and don't have more than 25% of our regions as supply bases
				// generate supply bases for the last two regions
				|| (region.ID == (c.second.ownedRegions.end() - 2)->ID && supplyHubProvinces.size() < (c.second.ownedRegions.size() / 4))) {
				// select a random gameprovince of the state
				auto y = *select_random(region.gameProvinces);
				for (auto& prov : region.gameProvinces) {
					if (prov.baseProvince->coastal) {
						// if this is a coastal region, the supply hub is a naval base as well
						y = prov;
						prov.attributeDoubles["naval_bases"] = 1;
						break;
					}
				}
				// save the province under the provinces ID
				supplyHubProvinces[y.ID] = y;
				navalBases[y.ID] = y.baseProvince->coastal;
				// get the distance between this supply hub and the capital
				auto distance = getDistance(capitalPosition, y.baseProvince->position, width);
				// save the distance under the province ID
				supplyHubs[distance] = y.ID;
				// save the distance
				distances.push_back(distance); // save distances to ensure ordering
			}
			for (auto gProv : region.gameProvinces) {
				gProvIDs.insert(gProv.ID);
			}
		}
		std::sort(distances.begin(), distances.end());
		for (const auto distance : distances) {
			vector<int> passthroughProvinceIDs;
			int attempts = 0;
			auto sourceNodeID = capitalProvince->ID;
			supplyNodeConnections.push_back({ sourceNodeID });
			do {
				attempts++;
				// the region we want to connect to the source
				auto destNodeID = supplyHubs[distance];
				if (sourceNodeID == capitalProvince->ID) {
					// we are at the start of the search
					// distance to capital
					auto tempDistance = distance;
					for (auto distance2 : distances) {
						// only check hubs that were already assigned
						if (distance2 < distance) {
							// distance is the distance between us and the capital
							// now find distance2, the distance between us and the other already assigned supply hubs
							auto dist3 = getDistance(scenGen.gameProvinces[supplyHubs[distance2]].baseProvince->position, scenGen.gameProvinces[supplyHubs[distance]].baseProvince->position, width);
							if (dist3 < tempDistance) {
								sourceNodeID = scenGen.gameProvinces[supplyHubs[distance2]].ID;
								tempDistance = dist3;
							}
						}
						supplyNodeConnections.back()[0] = sourceNodeID;
					}
				}
				else {
					// NOT at the start of the search, therefore sourceNodeID must be the last element of passThroughStates
					sourceNodeID = passthroughProvinceIDs.back();
				}
				// break if this is another landmass. We can't reach it anyway
				if (scenGen.gameProvinces[sourceNodeID].baseProvince->landMassID != scenGen.gameProvinces[destNodeID].baseProvince->landMassID)
					break;;
				// the origins position
				auto sourceNodePosition = scenGen.gameProvinces[sourceNodeID].baseProvince->position;
				// save the distance in a temp variable
				double tempMinDistance = width;
				auto closestID = INT_MAX;
				// now check every sourceNode neighbour for distance to destinationNode
				for (auto& neighbourGProvince : scenGen.gameProvinces[sourceNodeID].neighbours) {
					// check if this belongs to us
					if (gProvIDs.find(neighbourGProvince.ID) == gProvIDs.end())
						continue;
					bool cont = false;
					for (auto passThroughID : passthroughProvinceIDs) {
						if (passThroughID == neighbourGProvince.ID)
							cont = true;
					}
					if (cont) continue;
					// the distance to the sources neighbours
					auto nodeDistance = getDistance(scenGen.gameProvinces[destNodeID].baseProvince->position, neighbourGProvince.baseProvince->position, width);
					if (nodeDistance < tempMinDistance) {
						tempMinDistance = nodeDistance;
						closestID = neighbourGProvince.ID;
					}
				}
				if (closestID != INT_MAX) {
					// we found the next best state to go through in this direction
					passthroughProvinceIDs.push_back(closestID);
					// now save source
					sourceNodeID = passthroughProvinceIDs.back();
					if (passthroughProvinceIDs.back() == supplyHubs[distance]) {
						if (navalBases.at(supplyHubs[distance])) {
							connectedNavalBase = true;
						}
					}
				}
				// if we can't end this rail line, wrap up. Rails shouldn't be longer than 200 provinces anyway
				else if (attempts == 200) {
					// clean it up: if we can't reach our target, the railway must be cleared
					supplyNodeConnections.back().clear();
					passthroughProvinceIDs.clear();
					break;
				}
				else break;
			}
			// are we done? If no, find the next state, but the source is now the currently chosen neighbour
			while (passthroughProvinceIDs.back() != supplyHubs[distance] && attempts < 200);
			// we are done, as we have reached the destination node
			for (auto& passState : passthroughProvinceIDs) {
				supplyNodeConnections.back().push_back(passState);
			}
		}
		for (auto& pix : capitalProvince->baseProvince->pixels) {
			logistics.setColourAtIndex(pix, { 255,255,0 });
		}
		for (auto& supplyHubProvince : supplyHubProvinces) {
			for (auto& pix : supplyHubProvince.second.baseProvince->pixels) {
				logistics.setColourAtIndex(pix, { 0,255,0 });
			}
		}
	}
	for (auto& connection : supplyNodeConnections) {
		for (int i = 0; i < connection.size(); i++) {
			for (auto pix : scenGen.gameProvinces[connection[i]].baseProvince->pixels) {
				// don't overwrite capitals and supply nodes
				if (logistics.getColourAtIndex(pix) == Colour{ 255, 255, 0 } || logistics.getColourAtIndex(pix) == Colour{ 0, 255, 0 })
					continue;
				logistics.setColourAtIndex(pix, { 255,255,255 });
			}
		}
	}
	Bitmap::SaveBMPToFile(logistics, "Maps//logistics.bmp");
}

void Hoi4ScenarioGenerator::evaluateCountries(ScenarioGenerator& scenGen)
{
	std::cout << "HOI4: Evaluating Country Strength\n";
	std::map<int, vector<std::string>> strengthScores;
	for (auto& c : scenGen.countryMap) {
		auto totalIndustry = 0;
		auto totalPop = 0;
		auto maxIndustryID = 0;
		auto maxIndustryLevel = 0;
		for (auto& ownedRegion : c.second.ownedRegions) {
			auto regionIndustry = ownedRegion.attributeDoubles["civilianFactories"]
				+ ownedRegion.attributeDoubles["dockyards"]
				+ ownedRegion.attributeDoubles["armsFactories"];
			// always make the most industrious region the capital
			if (regionIndustry > maxIndustryLevel)
				c.second.capitalRegionID = ownedRegion.ID;
			totalIndustry += regionIndustry;
			totalPop += ownedRegion.attributeDoubles["population"];
		}
		strengthScores[totalIndustry + totalPop / 1'000'000].push_back(c.first);
		c.second.attributeDoubles["strengthScore"] = totalIndustry + totalPop / 1'000'000;
		// global
		totalWorldIndustry += totalIndustry;
	}
	int totalDeployedCountries = scenGen.numCountries - strengthScores[0].size();
	int numMajorPowers = totalDeployedCountries / 10;
	int numRegionalPowers = totalDeployedCountries / 3;
	int numWeakStates = totalDeployedCountries - numMajorPowers - numRegionalPowers;
	for (auto& scores : strengthScores) {
		for (auto& entry : scores.second) {
			if (scores.first > 0) {
				if (numWeakStates > weakPowers.size())
				{
					weakPowers.push_back(entry);
					scenGen.countryMap[entry].attributeStrings["rank"] = "weak";
				}
				else if (numRegionalPowers > regionalPowers.size()) {
					regionalPowers.push_back(entry);
					scenGen.countryMap[entry].attributeStrings["rank"] = "regional";
				}
				else {
					majorPowers.push_back(entry);
					scenGen.countryMap[entry].attributeStrings["rank"] = "major";
				}
			}
		}
	}
	generateCountryUnits(scenGen);
}

void Hoi4ScenarioGenerator::generateCountryUnits(ScenarioGenerator& scenGen)
{
	std::cout << "HOI4: Generating Country Unit Files\n";
	// read in different compositions
	auto compositionMajor = ParserUtils::getLines("resources\\hoi4\\history\\divisionCompositionMajor.txt");
	auto compositionRegional = ParserUtils::getLines("resources\\hoi4\\history\\divisionCompositionRegional.txt");
	auto compositionWeak = ParserUtils::getLines("resources\\hoi4\\history\\divisionCompositionWeak.txt");
	for (auto& c : scenGen.countryMap) {
		// determine the countries composition
		auto activeComposition = compositionWeak;
		if (c.second.attributeStrings["rank"] == "major")
			activeComposition = compositionMajor;
		else if (c.second.attributeStrings["rank"] == "regional")
			activeComposition = compositionRegional;
		// make room for unit values, as the index here is also the ID taken from the composition line
		c.second.attributeVectors["units"].resize(100);
		auto totalUnits = c.second.attributeDoubles["strengthScore"] / 5;
		for (auto& unit : activeComposition) {
			// get the composition line as numbers
			auto nums = ParserUtils::getNumbers(unit, ';', std::set<int>{});
			// now add the unit type. Share of total units * totalUnits
			c.second.attributeVectors["units"][nums[0]] = ((double)nums[1] / 100.0) * (double)totalUnits;
		}
	}
}

NationalFocus Hoi4ScenarioGenerator::buildFocus(vector<std::string> chainStep, Country& source, Country& target)
{
	auto x = chainStep[5];
	auto a = typeMapping["attack"];
	auto dateTokens = ParserUtils::getNumbers(chainStep[9], '-', std::set<int>{});
	NationalFocus nF(a, false, source.tag, target.tag, dateTokens);
	return nF;
}
/* checks all requirements for a national focus. Returns false if any requirement isn't fulfilled, else returns true*/
bool Hoi4ScenarioGenerator::fulfillsrequirements(vector<std::string> requirements, Country& source, Country& target)
{
	for (auto& requirement : requirements) {
		// need to check rank
		// first get the desired value
		auto value = ParserUtils::getBracketBlockContent(requirement, "rank");
		if (value != "") {
			if (target.attributeStrings["rank"] != value)
				return false; // targets rank is not right
		}
		value = ParserUtils::getBracketBlockContent(requirement, "ideology");
		if (value != "") {
			if (value == "any")
				continue; // fine, may target any ideology
			if (value == "same")
				if (target.attributeStrings["rulingParty"] != source.attributeStrings["rulingParty"])
					return false;
			if (value == "not")
				if (target.attributeStrings["rulingParty"] == source.attributeStrings["rulingParty"])
					return false;
		}
		value = ParserUtils::getBracketBlockContent(requirement, "location");
		if (value != "") {
			if (value == "any")
				continue; // fine, may target any ideology
			if (value == "neighbour") {
				if (source.neighbours.find(target.tag) == source.neighbours.end())
					return false;
			}
		}
	}
	return true;
}

void Hoi4ScenarioGenerator::evaluateCountryGoals(ScenarioGenerator& scenGen)
{
	std::cout << "HOI4: Generating Country Goals\n";
	std::vector<int> defDate{ 1,1,1936 };
	auto majorChains = ParserUtils::getLinesByID("resources\\hoi4\\history\\national_focus\\major_chains.txt");

	auto typeCounter = 0;
	for (auto& majorPower : majorPowers) {
		int chainID = 0;
		auto sourceS = scenGen.countryMap[majorPower].attributeStrings;
		auto sourceD = scenGen.countryMap[majorPower].attributeDoubles;
		for (auto chain : majorChains) {
			// evaluate whole chain (chain defined by ID)
			if (!chain.size())
				continue;
			// we need to save options for every chain step
			vector <vector<Country>> stepTargets;
			//stepTargets.resize(100); // leave some space

			for (auto chainFocus : chain) {
				// evaluate every single focus of that chain
				auto chainTokens = ParserUtils::getTokens(chainFocus, ';');
				int chainStep = stoi(chainTokens[1]);
				if (sourceS["rulingParty"] == chainTokens[4]) {
					stepTargets.resize(stepTargets.size() + 1);
					// source triggers this focus
					// split requirements
					auto targetRequirements = ParserUtils::getTokens(chainTokens[6], '+');
					for (auto& country : scenGen.countryMap) {
						// now check every country if it fulfills the target requirements
						if (fulfillsrequirements(targetRequirements, scenGen.countryMap[majorPower], country.second)) {
							stepTargets[chainStep].push_back(country.second);
						}
					}
				}
			}
			// now build the chain from the options
			if (stepTargets.size()) {
				std::cout << "Building focus" << std::endl;
				std::map<int, NationalFocus> fulfilledSteps;
				int stepIndex = -1;
				for (auto& targets : stepTargets) {
					stepIndex++;
					//std::cout << targets << std::endl;
					if (!targets.size())
						continue;
					auto target = *select_random(targets);
					auto focus = buildFocus(ParserUtils::getTokens(chain[stepIndex], ';'), scenGen.countryMap[majorPower], target);
					std::cout << focus << std::endl;
				}
			}
		}
	}
}

void Hoi4ScenarioGenerator::printStatistics()
{
	std::cout << "Total Industry: " << totalWorldIndustry << std::endl;
	std::cout << "Military Industry: " << militaryIndustry << std::endl;
	std::cout << "Civilian Industry: " << civilianIndustry << std::endl;
	std::cout << "Naval Industry: " << navalIndustry << std::endl;
	std::cout << "Total Aluminium: " << totalAluminium << std::endl;
	std::cout << "Total Chromium: " << totalChromium << std::endl;
	std::cout << "Total Rubber: " << totalRubber << std::endl;
	std::cout << "Total Oil: " << totalOil << std::endl;
	std::cout << "Total Steel: " << totalSteel << std::endl;
	std::cout << "Total Tungsten: " << totalTungsten << std::endl;
	std::cout << "World Population: " << worldPop << std::endl;
}



