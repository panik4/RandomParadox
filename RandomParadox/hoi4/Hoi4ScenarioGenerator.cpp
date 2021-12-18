#include "Hoi4ScenarioGenerator.h"



Hoi4ScenarioGenerator::Hoi4ScenarioGenerator(FastWorldGenerator& f, ScenarioGenerator sG) : scenGen(sG)
{
	this->f = f;
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
		std::cout << c.second.attributeStrings["fullName"] << std::endl;

		//c.second.attributeStrings["factionMajor"] = Data::getInstance().getRandomNumber(0, 1) ? "yes" : "no";

	}

}



