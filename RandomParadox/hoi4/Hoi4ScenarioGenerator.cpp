#include "Hoi4ScenarioGenerator.h"



Hoi4ScenarioGenerator::Hoi4ScenarioGenerator(ScenarioGenerator sG)
{
	this->random = Data::getInstance().random2;
}


Hoi4ScenarioGenerator::~Hoi4ScenarioGenerator()
{
}

void Hoi4ScenarioGenerator::generateStateSpecifics(ScenarioGenerator & scenGen)
{
	auto worldIndustry = 2000;
	for (auto& c : scenGen.countryMap)
	{
		for (auto& gameRegion : c.second.ownedRegions)
		{
			int totalPop = 0;
			double totalStateArea = 0;
			double totalDevFactor = 0;
			double totalPopFactor = 0;
			double worldArea = Data::getInstance().bitmapSize / 3; // roughly 1 third is land
			for (const auto& gameProv : gameRegion.gameProvinces)
			{
				totalDevFactor += gameProv.devFactor / (double)gameRegion.gameProvinces.size();
				totalPopFactor += gameProv.popFactor / (double)gameRegion.gameProvinces.size();
				totalStateArea += gameProv.baseProvince->pixels.size();
			}
			gameRegion.attributeDoubles["stateCategory"] = clamp((int)(totalPopFactor * 5.0 + totalDevFactor * 6.0), 0, 9);
			if (gameRegion.gameProvinces.size() == 1)
			{
				gameRegion.attributeDoubles["stateCategory"] = 1;
			}
			gameRegion.attributeDoubles["development"] = totalDevFactor;
			gameRegion.attributeDoubles["population"] = totalStateArea * 5000.0 * totalPopFactor;
			// count the total coastal provinces of this region
			auto totalCoastal = 0;
			for (auto& gameProv : gameRegion.gameProvinces)
			{
				if (gameProv.baseProvince->coastal)
				{
					totalCoastal++;
					gameProv.attributeDoubles["naval_bases"] = Data::getInstance().getRandomNumber(1, 5);
				}
				else {
					gameProv.attributeDoubles["naval_bases"] = 0;
				}
			}
			auto stateIndustry = (totalStateArea / worldArea) * totalPopFactor * worldIndustry;
			if (totalCoastal > 0)
			{
				gameRegion.attributeDoubles["dockyards"] = clamp((int)round(stateIndustry*(0.33)), 0, 4);
				gameRegion.attributeDoubles["civilianFactories"] = clamp((int)round(stateIndustry*(0.34)), 0, 8);
				gameRegion.attributeDoubles["armsFactories"] = clamp((int)round(stateIndustry*(0.33)), 0, 4);
			}
			else {
				gameRegion.attributeDoubles["civilianFactories"] = clamp((int)round(stateIndustry*(0.67)), 0, 8);
				gameRegion.attributeDoubles["armsFactories"] = clamp((int)round(stateIndustry*(0.33)), 0, 4);
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

void Hoi4ScenarioGenerator::evaluateCountries(ScenarioGenerator & scenGen)
{
	std::cout << "HOI4: Evaluating Country Strength\n";
	std::map<int, vector<std::string>> strengthScores;
	for (auto& c : scenGen.countryMap)
	{
		auto totalIndustry = 0;
		auto totalPop = 0;
		for (auto& ownedRegion : c.second.ownedRegions)
		{
			totalIndustry += ownedRegion.attributeDoubles["civilianFactories"];
			totalIndustry += ownedRegion.attributeDoubles["dockyards"];
			totalIndustry += ownedRegion.attributeDoubles["armsFactories"];
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



