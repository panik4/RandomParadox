#include "Hoi4ScenarioGenerator.h"


Hoi4ScenarioGenerator::Hoi4ScenarioGenerator()
{
	this->random = Data::getInstance().random2;
}


Hoi4ScenarioGenerator::~Hoi4ScenarioGenerator()
{
}

void Hoi4ScenarioGenerator::generateStateResources(ScenarioGenerator & scenGen)
{
	for (auto& c : scenGen.countryMap) {
		for (auto& gameRegion : c.second.ownedRegions) {
			for (auto& resource : resources)
			{
				auto chance = resource.second[2];
				if (random() % 100 < chance*100.0) {
					// calc total of this resource
					auto totalOfResource = resource.second[1] * resource.second[0];
					// more per selected state if the chance is lower
					double averagePerState = (totalOfResource / (double)landStates) * (1.0 / chance);
					// range 1 to (2 times average - 1)
					double value = 1 + random() % (int)((2.0*averagePerState));
					gameRegion.attributeDoubles[resource.first] = value;
				}
			}
		}
	}
}

void Hoi4ScenarioGenerator::generateStateSpecifics(ScenarioGenerator & scenGen)
{
	auto worldIndustry = 1216;
	for (auto& c : scenGen.countryMap) {
		for (auto& gameRegion : c.second.ownedRegions) {
			// count the number of land states for resource generation
			landStates++;
			int totalPop = 0;
			double totalStateArea = 0;
			double totalDevFactor = 0;
			double totalPopFactor = 0;
			double worldArea = Data::getInstance().bitmapSize / 3; // roughly 1 third is land
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
			gameRegion.attributeDoubles["population"] = totalStateArea * 5000.0 * totalPopFactor;
			// count the total coastal provinces of this region
			auto totalCoastal = 0;
			for (auto& gameProv : gameRegion.gameProvinces) {
				if (gameProv.baseProvince->coastal) {
					totalCoastal++;
					gameProv.attributeDoubles["naval_bases"] = Data::getInstance().getRandomNumber(1, 5);
				}
				else {
					gameProv.attributeDoubles["naval_bases"] = 0;
				}
			}
			auto stateIndustry = (totalStateArea / worldArea) * totalPopFactor * worldIndustry;
			if (totalCoastal > 0) {
				gameRegion.attributeDoubles["dockyards"] = clamp((int)round(stateIndustry*(0.17)), 0, 4);
				gameRegion.attributeDoubles["civilianFactories"] = clamp((int)round(stateIndustry*(0.68)), 0, 8);
				gameRegion.attributeDoubles["armsFactories"] = clamp((int)round(stateIndustry*(0.16)), 0, 4);
			}
			else {
				gameRegion.attributeDoubles["civilianFactories"] = clamp((int)round(stateIndustry*(0.75)), 0, 8);
				gameRegion.attributeDoubles["armsFactories"] = clamp((int)round(stateIndustry*(0.25)), 0, 4);
				gameRegion.attributeDoubles["dockyards"] = 0;
			}
		}
	}

}

void Hoi4ScenarioGenerator::generateCountrySpecifics(ScenarioGenerator & scenGen, std::map<std::string, Country>& countries)
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
	for (auto& c : countries)
	{
		c.second.attributeStrings["gfxCulture"] = *select_random(gfxCultures);
	}
	for (auto& c : countries)
	{
		vector<double> popularities{};
		double totalPop = 0;
		for (int i = 0; i < 4; i++)
		{
			popularities.push_back(Data::getInstance().getRandomNumber(1, 100));
			totalPop += popularities[i];
		}
		auto sumPop = 0;
		for (int i = 0; i < 4; i++)
		{
			popularities[i] = popularities[i] / totalPop * 100;
			sumPop += popularities[i];
			int offset = 0;
			if (i == 3 && sumPop < 100)
			{
				offset = 100 - sumPop;
			}
			c.second.attributeDoubles[ideologies[i]] = popularities[i] + offset;
		}
		c.second.attributeStrings["rulingParty"] = ideologies[Data::getInstance().getRandomNumber(0, ideologies.size())];
		if (c.second.attributeStrings["rulingParty"] == "democratic")
			c.second.attributeDoubles["allowElections"] = 1;
		else if (c.second.attributeStrings["rulingParty"] == "neutrality")
			c.second.attributeDoubles["allowElections"] = Data::getInstance().getRandomNumber(0, 1);
		else
			c.second.attributeDoubles["allowElections"] = 0;
		c.second.attributeStrings["fullName"] = scenGen.nG.modifyWithIdeology(c.second.attributeStrings["rulingParty"], c.second.name, c.second.adjective);

		//c.second.attributeStrings["factionMajor"] = Data::getInstance().getRandomNumber(0, 1) ? "yes" : "no";

	}

}

void Hoi4ScenarioGenerator::generateLogistics(ScenarioGenerator & scenGen)
{
	std::cout << "Building rail networks\n";
	auto width = Data::getInstance().width;
	//auto t = Data::getInstance().findBitmapByKey(terr)
	Bitmap logistics = Data::getInstance().findBitmapByKey("countries");//(width, Data::getInstance().height, 24);
	// first create supply hubs
	for (auto& c : scenGen.countryMap) {
		// ID, distance
		std::map<double, int> supplyHubs;
		// add capital
		//supplyHubs[0.0] = c.second.capitalRegionID;
		auto capitalPosition = scenGen.gameRegions[c.second.capitalRegionID].position;
		auto capitalProvince = select_random(scenGen.gameRegions[c.second.capitalRegionID].gameProvinces);
		vector<double> distances;
		// region ID, provinceID
		std::map<int, GameProvince> supplyHubProvinces;
		for (auto& region : c.second.ownedRegions) {
			if (region.attributeDoubles["stateCategory"] > 7 && region.ID!=c.second.capitalRegionID) {
				auto distance = getDistance(capitalPosition, region.position, width);
				supplyHubs[distance] = region.ID;
				distances.push_back(distance); // save distances to ensure ordering
				supplyHubProvinces[region.ID] = *select_random(scenGen.gameRegions[region.ID].gameProvinces);
			}
		}
		// a list of connections: {sourceHub, destHub, provinces the rails go through}
		vector<vector<int>> connections;
		for (auto distance : distances) {
			vector<int> passthroughStateIDs;
			int attempts = 0;
			auto sourceNodeID = c.second.capitalRegionID;
			do {
				attempts++;
				if (sourceNodeID == c.second.capitalRegionID) {
					// we are at the start of the search
					auto tempDist2 = width;
					for (auto distance2 : distances) {
						// only check hubs that were already assigned
						if (distance2 < distance) {
							// distance is the distance between us and the capital
							// now find distance2, the distance between us and the other álready assigned supply hubs
							if (getDistance(scenGen.gameRegions[supplyHubs[distance2]].position, scenGen.gameRegions[supplyHubs[distance]].position, width))
								sourceNodeID = scenGen.gameRegions[supplyHubs[distance2]].ID;
						}
					}
				}
				else {
					// NOT at the start of the search, therefore sourceNodeID must be the last element of passThroughStates
					sourceNodeID = passthroughStateIDs.back();
				}
				// the origins position
				auto sourceNodePosition = scenGen.gameRegions[sourceNodeID].position;
				// the region we want to connect to the source
				auto destNodeID = supplyHubs[distance];
				// save the distance in a temp variable
				double tempMinDistance = Data::getInstance().width;
				auto closestID = 0;
				// now check every sourceNode neighbour for distance to destinationNode
				for (auto& neighbourID : scenGen.gameRegions[sourceNodeID].neighbours) {
					// check if this belongs to us
					if (scenGen.gameRegions[neighbourID].owner != c.first)
						continue;
					// the distance to the sources neighbours
					auto nodeDistance = getDistance(scenGen.gameRegions[destNodeID].position, scenGen.gameRegions[neighbourID].position, width);
					if (nodeDistance < tempMinDistance) {
						tempMinDistance = nodeDistance;
						closestID = neighbourID;
					}
				}
				// we found the next best state to go through in this direction
				passthroughStateIDs.push_back(closestID);
				sourceNodeID = passthroughStateIDs.back();
			}
			// are we done? If no, find the next state, but the source is now the currently chosen neighbour
			while (passthroughStateIDs.back() != supplyHubs[distance] && attempts < 1000);
			// we are done, as we have reached the destination node
			connections.push_back({ c.second.capitalRegionID, supplyHubs[distance] });
			for (auto& passState : passthroughStateIDs) {
				connections[connections.size() - 1].push_back(passState);
			}
		}


		// now debug draw the state paths on a map
		for (auto& connection : connections) {
			for (int i = 2; i < connection.size(); i++) {
				for (auto& province : scenGen.gameRegions[connection[i]].gameProvinces) {
					for (auto pix : province.baseProvince->pixels) {
						if (province.ID != (*capitalProvince).ID) {
							logistics.setColourAtIndex(pix, { 255,255,255 });
						}
					}
				}
			}
		}
		for (auto& pix : capitalProvince->baseProvince->pixels) {
			logistics.setColourAtIndex(pix, { 255,255,0 });
		}
		for (auto& supplyHubProvince : supplyHubProvinces)
			for (auto& pix : supplyHubProvince.second.baseProvince->pixels) {
				logistics.setColourAtIndex(pix, { 0,255,0 });
			}

		Bitmap::SaveBMPToFile(logistics, "Maps//logistics.bmp");
	}
}

void Hoi4ScenarioGenerator::evaluateCountries(ScenarioGenerator & scenGen)
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
			if (regionIndustry > maxIndustryLevel)
				c.second.capitalRegionID = ownedRegion.ID;
			totalIndustry += regionIndustry;
			totalPop += ownedRegion.attributeDoubles["population"];
		}
		strengthScores[totalIndustry + totalPop / 1000000].push_back(c.first);
		c.second.attributeDoubles["strengthScore"] = totalIndustry + totalPop / 1000000;

	}

	int totalDeployedCountries = 100 - strengthScores[0].size();
	int numMajorPowers = totalDeployedCountries / 10;
	int numRegionalPowers = totalDeployedCountries / 3;
	int numWeakStates = totalDeployedCountries - numMajorPowers - numRegionalPowers;
	for (auto& scores : strengthScores)
	{
		for (auto& entry : scores.second)
		{
			if (scores.first > 0)
			{
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

void Hoi4ScenarioGenerator::generateCountryUnits(ScenarioGenerator & scenGen)
{
	std::cout << "HOI4: Generating Country Unit Files\n";
	// read in different compositions
	auto compositionMajor = ParserUtils::getLines("resources\\hoi4\\history\\divisionCompositionMajor.txt");
	auto compositionRegional = ParserUtils::getLines("resources\\hoi4\\history\\divisionCompositionRegional.txt");
	auto compositionWeak = ParserUtils::getLines("resources\\hoi4\\history\\divisionCompositionWeak.txt");

	for (auto& c : scenGen.countryMap)
	{
		// determine the countries composition
		auto activeComposition = compositionWeak;
		if (c.second.attributeStrings["rank"] == "major")
			activeComposition = compositionMajor;
		else if (c.second.attributeStrings["rank"] == "regional")
			activeComposition = compositionRegional;
		// make room for unit values, as the index here is also the ID taken from the composition line
		c.second.attributeVectors["units"].resize(100);
		auto totalUnits = c.second.attributeDoubles["strengthScore"] / 5;
		for (auto& unit : activeComposition)
		{
			// get the composition line as numbers
			auto nums = ParserUtils::getNumbers(unit, ';', std::set<int>{});
			// now add the unit type. Share of total units * totalUnits
			c.second.attributeVectors["units"][nums[0]] = ((double)nums[1] / 100.0) * (double)totalUnits;
		}
	}
}

NationalFocus Hoi4ScenarioGenerator::buildFocus(vector<std::string> chainStep, Country & source, Country & target)
{
	auto x = chainStep[5];
	auto a = typeMapping["attack"];
	auto dateTokens = ParserUtils::getNumbers(chainStep[9], '-', std::set<int>{});
	NationalFocus nF(a, false, source.tag, target.tag, dateTokens);
	return nF;
}

bool Hoi4ScenarioGenerator::fulfillsrequirements(vector<std::string> requirements, Country& source, Country& target)
{
	for (auto& requirement : requirements)
	{
		// need to check rank
		// first get the desired value
		auto value = ParserUtils::getBracketBlockContent(requirement, "rank");
		if (value != "")
		{
			if (target.attributeStrings["rank"] != value)
				return false; // targets rank is not right
		}
		value = ParserUtils::getBracketBlockContent(requirement, "ideology");
		if (value != "")
		{
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
		if (value != "")
		{
			if (value == "any")
				continue; // fine, may target any ideology
			if (value == "neighbour")
			{
				if (source.neighbours.find(target.tag) == source.neighbours.end())
					return false;
			}
		}
	}

	return true;
}

void Hoi4ScenarioGenerator::evaluateCountryGoals(ScenarioGenerator & scenGen)
{
	std::cout << "HOI4: Generating Country Goals\n";
	std::vector<int> defDate{ 1,1,1936 };
	auto majorChains = ParserUtils::getLinesByID("resources\\hoi4\\history\\national_focus\\major_chains.txt");

	auto typeCounter = 0;
	for (auto& majorPower : majorPowers)
	{
		int chainID = 0;
		auto sourceS = scenGen.countryMap[majorPower].attributeStrings;
		auto sourceD = scenGen.countryMap[majorPower].attributeDoubles;
		for (auto chain : majorChains)
		{
			// evaluate whole chain (chain defined by ID)
			if (!chain.size())
				continue;
			// we need to save options for every chain step
			vector <vector<Country>> stepTargets;
			//stepTargets.resize(100); // leave some space

			for (auto chainFocus : chain)
			{
				// evaluate every single focus of that chain
				auto chainTokens = ParserUtils::getTokens(chainFocus, ';');
				int chainStep = stoi(chainTokens[1]);
				if (sourceS["rulingParty"] == chainTokens[4])
				{
					stepTargets.resize(stepTargets.size() + 1);
					// source triggers this focus
					// split requirements
					auto targetRequirements = ParserUtils::getTokens(chainTokens[6], '+');
					for (auto& country : scenGen.countryMap)
					{
						// now check every country if it fulfills the target requirements
						if (fulfillsrequirements(targetRequirements, scenGen.countryMap[majorPower], country.second))
						{
							stepTargets[chainStep].push_back(country.second);
						}
					}
				}
			}
			// now build the chain from the options
			if (stepTargets.size())
			{
				cout << "Building focus" << std::endl;
				std::map<int, NationalFocus> fulfilledSteps;
				int stepIndex = -1;
				for (auto& targets : stepTargets)
				{
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


	//for (auto& majorPower : majorPowers)
	//{
	//	auto sourceS = scenGen.countryMap[majorPower].attributeStrings;
	//	auto sourceD = scenGen.countryMap[majorPower].attributeDoubles;
	//	for (auto& neighbour : scenGen.countryMap[majorPower].neighbours)
	//	{
	//		auto targetS = scenGen.countryMap[neighbour].attributeStrings;
	//		auto targetD = scenGen.countryMap[neighbour].attributeDoubles;
	//		if (sourceD["strengthScore"] < 0.66 * targetD["strengthScore"])
	//		{
	//			// source is significantly weaker
	//			if (sourceS["rulingParty"] != targetS["rulingParty"])
	//			{

	//			}
	//		}
	//		else if (sourceD["strengthScore"] < 1.33 * targetD["strengthScore"]) {
	//			// source is about equal strength
	//			if (sourceS["rulingParty"] != targetS["rulingParty"])
	//			{

	//			}
	//		}
	//		else {
	//			if (sourceS["rulingParty"] != targetS["rulingParty"])
	//			{
	//				// bool default, std::string source, std::string dest, std::vector<int> date
	//				std::cout << majorPower << " Attacks " << neighbour << std::endl;
	//				//NationalFocus::FocusType ftype,

	//				NationalFocus f(focusID++, NationalFocus::FocusType::attack, false, majorPower, neighbour, defDate);
	//				foci.push_back(f);
	//				warFoci.push_back(f);
	//			}
	//		}
	//	}
	//	for (auto &otherMajor : majorPowers)
	//	{
	//		if (otherMajor == majorPower)
	//			continue;
	//		auto targetS = scenGen.countryMap[otherMajor].attributeStrings;
	//		auto targetD = scenGen.countryMap[otherMajor].attributeDoubles;
	//		if (sourceS["rulingParty"] == "fascism" || sourceS["rulingParty"] == "communism")
	//		{
	//			if (targetS["rulingParty"] == sourceS["rulingParty"])
	//			{
	//				// establish dominance or ally
	//				NationalFocus f(focusID++, NationalFocus::FocusType::attack, false, majorPower, otherMajor, { 1,2,1936 });
	//				NationalFocus f2(focusID++, NationalFocus::FocusType::ally, false, majorPower, otherMajor, { 1,2,1936 });
	//				vector<NationalFocus> alt{ f,f2 };
	//				NationalFocus::makeAlternative(alt);
	//				foci.push_back(alt[0]);
	//				foci.push_back(alt[1]);
	//			}
	//		}
	//	}
	//}
}



