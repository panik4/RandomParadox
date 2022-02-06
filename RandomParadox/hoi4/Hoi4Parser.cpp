#include "Hoi4Parser.h"



Hoi4Parser::Hoi4Parser()
{
}


Hoi4Parser::~Hoi4Parser()
{
}
void Hoi4Parser::dumpAdj(std::string path)
{
	std::cout << "HOI4 Parser: Map: Writing Adjacencies\n";
	// From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
	// empty file for now
	std::string content;
	content.append("From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment");
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpAirports(std::string path, const vector<Region>& regions)
{
	std::cout << "HOI4 Parser: Map: Building Airfields\n";
	std::string content;
	for (auto region : regions)
	{
		if (region.sea)
			continue;
		content.append(to_string(region.ID + 1));
		content.append("={");
		content.append(to_string(region.provinces[0]->ID + 1));
		content.append(" }\n");
	}
	pU::writeFile(path, content);
}

std::string Hoi4Parser::getBuildingLine(std::string type, Region& region, bool coastal, Bitmap& heightmap)
{
	auto prov = *select_random(region.provinces);
	auto pix = 0;
	if (coastal)
	{
		while (!prov->coastal)
		{
			prov = *select_random(region.provinces);
		}
		pix = *select_random(prov->coastalPixels);
	}
	else {
		while (prov->isLake)
		{
			prov = *select_random(region.provinces);
		}
		pix = *select_random(prov->pixels);
	}

	auto widthPos = (pix % Data::getInstance().width);
	auto heightPos = /*Data::getInstance().height -*/ (pix / Data::getInstance().width);
	std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string((double)heightmap.getColourAtIndex(pix).getRed() / 10.0), to_string(heightPos), to_string((float)-1.57), "0" };
	return pU::csvFormat(arguments, ';', false);
}

// places building positions
void Hoi4Parser::dumpBuildings(std::string path, const vector<Region>& regions)
{
	std::cout << "HOI4 Parser: Map: Constructing Factories\n";
	auto heightmap = Data::getInstance().findBitmapByKey("heightmap");


	vector<std::string> buildingTypes{ "arms_factory", "industrial_complex", "air_base",
		"bunker", "coastal_bunker", "dockyard", "naval_base", "anti_air_building",
		"synthetic_refinery", "nuclear_reactor", "rocket_site", "radar_station", "fuel_silo" };
	std::string content;
	auto random = Data::getInstance().random2;
	// stateId; type; pixelX, rotation??, pixelY, rotation??, 0??}
	// 1; arms_factory; 2946.00; 11.63; 1364.00; 0.45; 0
	for (auto region : regions)
	{
		if (region.sea)
			continue;
		bool coastal = false;
		for (auto prov : region.provinces)
		{
			if (prov->coastal)
			{
				coastal = true;
			}
		}
		for (auto type : buildingTypes)
		{
			if (type == "arms_factory" || type == "industrial_complex")
			{
				for (int i = 0; i < 6; i++)
				{
					content.append(getBuildingLine(type, region, false, heightmap));
				}
			}
			else if (type == "bunker")
			{
				for (auto prov : region.provinces)
				{
					if (!prov->isLake && !prov->sea)
					{
						auto pix = *select_random(prov->pixels);
						auto widthPos = (pix % Data::getInstance().width);
						auto heightPos = /*Data::getInstance().height - */(pix / Data::getInstance().width);
						std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string((double)heightmap.getColourAtIndex(pix).getRed() / 10.0), to_string(heightPos), to_string(0.5), "0" };
						content.append(pU::csvFormat(arguments, ';', false));
					}
				}
			}
			else if (type == "anti_air_building")
			{
				for (int i = 0; i < 3; i++)
				{
					content.append(getBuildingLine(type, region, false, heightmap));
				}
			}
			else if (type == "coastal_bunker" || type == "naval_base")
			{
				for (auto prov : region.provinces)
				{
					if (prov->coastal)
					{
						auto pix = *select_random(prov->coastalPixels);
						uint32_t ID = 0;
						if (type == "naval_base")
						{
							// find the ocean province this coastal building is next to
							for (auto neighbour : prov->adjProv)
							{
								if (neighbour->sea)
								{
									for (auto provPix : neighbour->pixels)
									{
										if (getDistance(provPix, pix, Data::getInstance().width, 0) < 2.0)
										{
											ID = neighbour->ID;
										}
									}
								}
							}
						}
						auto widthPos = (pix % Data::getInstance().width);
						auto heightPos = /*Data::getInstance().height -*/ (pix / Data::getInstance().width);
						std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos),  to_string((double)heightmap.getColourAtIndex(pix).getRed() / 10.0), to_string(heightPos), to_string(0.5), to_string(ID + 1) };
						content.append(pU::csvFormat(arguments, ';', false));
					}
				}
			}
			else if (type == "dockyard")
			{
				if (coastal)
				{
					content.append(getBuildingLine(type, region, coastal, heightmap));
				}
			}
			else {
				{
					content.append(getBuildingLine(type, region, false, heightmap));
				}
			}
		}
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpContinents(std::string path, const vector<Continent>& continents)
{
	std::cout << "HOI4 Parser: Map: Writing Continents\n";
	std::string content{ "continents = {\n" };

	for (auto continent : continents)
	{
		content.append("\t");
		content.append(to_string(continent.ID + 1));
		content.append("\n");
	}
	content.append("}\n");
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpDefinition(std::string path, vector<GameProvince>& provinces)
{
	std::cout << "HOI4 Parser: Map: Defining Provinces\n";
	// province id; r value; g value; b value; province type (land/sea/lake); coastal (true/false); terrain (plains/hills/urban/etc. Defined for land or sea provinces in common/terrain); continent (int)
	// 0;0;0;0;land;false;unknown;0

	// terraintypes: ocean, lakes, forest, hills, mountain, plains, urban, jungle, marsh, desert, water_fjords, water_shallow_sea, water_deep_ocean
	// TO DO: properly map terrain types from climate
	//Bitmap typeMap(512, 512, 24);
	std::string content{ "0;0;0;0;land;false;unknown;0\n" };
	for (auto prov : provinces)
	{
		auto seaType = prov.baseProvince->sea ? "sea" : "land";
		auto coastal = prov.baseProvince->coastal ? "true" : "false";
		if (prov.baseProvince->sea)
		{
			for (auto prov2 : prov.baseProvince->adjProv)
			{
				if (!prov2->sea)
					coastal = "true";
			}
		}
		std::string terraintype;
		if (prov.baseProvince->sea)
			terraintype = "ocean";
		else
			terraintype = prov.terrainType;
		if (prov.baseProvince->isLake)
		{
			terraintype = "lakes";
			seaType = "lake";
		}
		std::vector<std::string> arguments{ to_string(prov.baseProvince->ID + 1),
			to_string(prov.baseProvince->colour.getRed()),
			to_string(prov.baseProvince->colour.getGreen()),
			to_string(prov.baseProvince->colour.getBlue()),
			seaType, coastal, terraintype,
			to_string(prov.baseProvince->sea || prov.baseProvince->isLake ? 0 : prov.baseProvince->continentID + 1) // 0 is for sea, no continent
		};
		content.append(pU::csvFormat(arguments, ';', false));
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpRocketSites(std::string path, const vector<Region>& regions)
{
	std::cout << "HOI4 Parser: Map: Launching Rockets\n";
	std::string content;
	// regionId={provId }
	for (auto region : regions)
	{
		if (region.sea)
			continue;
		content.append(to_string(region.ID + 1));
		content.append("={");
		content.append(to_string(region.provinces[0]->ID + 1));
		content.append(" }\n");
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpUnitStacks(std::string path, const vector<Province*> provinces)
{
	std::cout << "HOI4 Parser: Map: Remilitarizing the Rhineland\n";
	// 1;0;3359.00;9.50;1166.00;0.00;0.08
	// provID, neighbour?, xPos, zPos yPos, rotation(3=north, 0=south, 1.5=east,4,5=west), ??
	// provID, xPos, ~10, yPos, ~0, 0,5
	// for each neighbour add move state in the direction of the neighbour. 0 might be stand still
	std::string content{ "" };
	for (auto prov : provinces)
	{
		int position = 0;
		auto pix = *select_random(prov->pixels);
		auto widthPos = pix % Data::getInstance().width;
		auto heightPos = pix / Data::getInstance().width;
		std::vector<std::string> arguments{ to_string(prov->ID + 1), to_string(position), to_string(widthPos), to_string(1), to_string(heightPos), to_string(0.0), "0.0" };
		content.append(pU::csvFormat(arguments, ';', false));
		for (auto neighbour : prov->adjProv)
		{
			position++;
			double angle;
			auto nextPos = prov->getPositionBetweenProvinces(*neighbour, Data::getInstance().width, angle);
			angle += 1.57;
			auto widthPos = nextPos % Data::getInstance().width;
			auto heightPos = nextPos / Data::getInstance().width;
			std::vector<std::string> arguments{ to_string(prov->ID + 1), to_string(position), to_string(widthPos), to_string(15), to_string(heightPos), to_string(angle), "0.0" };
			content.append(pU::csvFormat(arguments, ';', false));
		}
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpWeatherPositions(std::string path, const vector<Region>& regions)
{
	std::cout << "HOI4 Parser: Map: Creating Storms\n";
	// 1; 2781.24; 9.90; 1571.49; small
	std::string content{ "" };
	auto random = Data::getInstance().random2;
	// stateId; pixelX; rotation??; pixelY; rotation??; size
	// 1; arms_factory; 2946.00; 11.63; 1364.00; 0.45; 0
	for (auto region : regions)
	{
		auto prov = *select_random(region.provinces);
		auto pix = *select_random(prov->pixels);
		auto widthPos = pix % Data::getInstance().width;
		auto heightPos = pix / Data::getInstance().width;
		std::vector<std::string> arguments{ to_string(region.ID + 1), to_string(widthPos), to_string(9.90), to_string(heightPos), "small" };
		content.append(pU::csvFormat(arguments, ';', false));
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpAdjacencyRules(std::string path)
{
	std::cout << "HOI4 Parser: Map: Writing Adjacency Rules\n";
	std::string content{ "" };
	// empty for now
	pU::writeFile(path, content);
}

// awful, just awful
void Hoi4Parser::dumpStrategicRegions(std::string path, const vector<Region>& regions)
{
	std::cout << "HOI4 Parser: Map: Drawing Strategic Regions\n";
	auto templateContent = pU::readFile("resources\\hoi4\\map\\strategic_region.txt");

	for (auto region : regions)
	{
		sort(region.provinces.begin(), region.provinces.end());
		region.provinces.erase(unique(region.provinces.begin(), region.provinces.end()), region.provinces.end());
		std::string provString{ "" };
		for (auto prov : region.provinces)
		{
			provString.append(to_string(prov->ID + 1));
			provString.append(" ");
		}
		auto content = templateContent;
		pU::replaceOccurences(content, "templateID", to_string(region.ID + 1));
		pU::replaceOccurences(content, "template_provinces", provString);
		pU::writeFile(path + "\\" + to_string(region.ID + 1) + ".txt", content);
	}
}

void Hoi4Parser::dumpSupplyAreas(std::string path, const  vector<Region>& regions)
{
	std::cout << "HOI4 Parser: Map: Supplying the Troops\n";
	auto templateContent = pU::readFile("resources\\hoi4\\map\\SupplyArea.txt");
	for (auto region : regions)
	{
		if (region.sea)
			continue;
		auto content = templateContent;
		pU::replaceOccurences(content, "templateID", to_string(region.ID + 1));
		pU::replaceOccurences(content, "templateState", to_string(region.ID + 1));
		pU::writeFile(path + "\\" + to_string(region.ID + 1) + "-SupplyArea.txt", content);
	}
}

void Hoi4Parser::dumpSupply(std::string path, const vector<vector<int>> supplyNodeConnections)
{
	std::string supplyNodes = "";
	std::string railways = "";
	std::set<int> nodes;
	for (auto& connection : supplyNodeConnections) {
		if (connection.size() <= 1)
			continue;
		nodes.insert(connection[0]);
		nodes.insert(connection.back());
		railways += "1 ";
		railways += to_string(connection.size());
		railways += " ";
		for (auto prov : connection) {
			railways += to_string(prov+1);
			railways += " ";
		}
		railways += "\n";
	}
	for (auto node : nodes) {
		supplyNodes.append("1 " + to_string(node+1) + "\n");
	}
	ParserUtils::writeFile(path + "supply_nodes.txt", supplyNodes);
	ParserUtils::writeFile(path + "railways.txt", railways);
}

void Hoi4Parser::dumpStates(std::string path, std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: History: Drawing State Borders\n";
	auto templateContent = pU::readFile("resources\\hoi4\\history\\state.txt");
	vector<std::string> stateCategories{ "wasteland", "small_island", "pastoral", "rural", "town", "large_town", "city", "large_city", "metropolis", "megalopolis" };
	for (auto& country : countries)
	{
		for (auto& region : country.second.ownedRegions)
		{
			auto baseRegion = region.baseRegion;
			if (baseRegion.sea)
				continue;
			sort(baseRegion.provinces.begin(), baseRegion.provinces.end());
			baseRegion.provinces.erase(unique(baseRegion.provinces.begin(), baseRegion.provinces.end()), baseRegion.provinces.end());
			std::string provString{ "" };
			for (auto prov : baseRegion.provinces)
			{
				provString.append(to_string(prov->ID + 1));
				provString.append(" ");
			}
			auto content = templateContent;
			pU::replaceOccurences(content, "templateID", to_string(baseRegion.ID + 1));
			pU::replaceOccurences(content, "template_provinces", provString);
			pU::replaceOccurences(content, "templateOwner", country.first);
			pU::replaceOccurences(content, "templateInfrastructure", to_string(1 + (int)(region.attributeDoubles["development"] * 4.0)));
			pU::replaceOccurences(content, "templateAirbase", to_string(0));
			pU::replaceOccurences(content, "templateCivilianFactory", to_string((int)region.attributeDoubles["civilianFactories"]));
			pU::replaceOccurences(content, "templateArmsFactory", to_string((int)region.attributeDoubles["armsFactories"]));
			pU::replaceOccurences(content, "templatePopulation", to_string((int)region.attributeDoubles["population"]));
			pU::replaceOccurences(content, "templateStateCategory", stateCategories[(int)region.attributeDoubles["stateCategory"]]);
			std::string navalBaseContent = "";
			for (auto& gameProv : region.gameProvinces)			{
				if (gameProv.attributeDoubles["naval_bases"] > 0)				{
					navalBaseContent += to_string(gameProv.ID + 1) + " = {\n\t\t\t\tnaval_base = " + to_string((int)gameProv.attributeDoubles["naval_bases"]) + "\n\t\t\t}\n\t\t\t";
				}
			}
			pU::replaceOccurences(content, "templateNavalBases", navalBaseContent);
			if (region.attributeDoubles["dockyards"] > 0)
				pU::replaceOccurences(content, "templateDockyards", to_string((int)region.attributeDoubles["dockyards"]));
			else
				pU::replaceOccurences(content, "dockyard = templateDockyards", "");

			// resources
			for (auto resource : vector<std::string>{ "aluminium", "chromium", "oil", "rubber", "steel", "tungsten" }) {
				pU::replaceOccurences(content, "template" + resource, to_string((int)region.attributeDoubles[resource]));
			}
			pU::writeFile(path + "\\" + to_string(baseRegion.ID + 1) + ".txt", content);
		}
	}
}
void Hoi4Parser::dumpFlags(std::string path, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: Gfx: Printing Flags\n";
	for (auto country : countries)
	{
		TextureWriter::writeTGA(country.second.flag.width, country.second.flag.height, country.second.flag.getFlag(), path + country.first + ".tga");
		TextureWriter::writeTGA(country.second.flag.width / 2, country.second.flag.height / 2, country.second.flag.resize(country.second.flag.width / 2, country.second.flag.height / 2), path + "\\medium\\" + country.first + ".tga");
		TextureWriter::writeTGA(10, 7, country.second.flag.resize(10, 7), path + "\\small\\" + country.first + ".tga");
	}
}

void Hoi4Parser::writeHistoryCountries(std::string path, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: History: Writing Country History\n";
	auto content = pU::readFile("resources\\hoi4\\history\\country_template.txt");
	for (auto country : countries)
	{
		auto tempPath = path + country.first + " - " + country.second.name + ".txt";
		auto countryText = content;
		auto capitalID = 1;
		if (country.second.ownedRegions.size())
			capitalID = (*select_random(country.second.ownedRegions)).ID + 1;
		pU::replaceOccurences(countryText, "templateCapital", to_string(capitalID));
		pU::replaceOccurences(countryText, "templateTag", country.first);
		pU::replaceOccurences(countryText, "templateParty", country.second.attributeStrings["rulingParty"]);
		std::string electAllowed = country.second.attributeDoubles["allowElections"] ? "yes" : "no";
		pU::replaceOccurences(countryText, "templateAllowElections", electAllowed);
		pU::replaceOccurences(countryText, "templateDemPop", to_string(country.second.attributeDoubles["democratic"]));
		pU::replaceOccurences(countryText, "templateFasPop", to_string(country.second.attributeDoubles["fascism"]));
		pU::replaceOccurences(countryText, "templateComPop", to_string(country.second.attributeDoubles["communism"]));
		pU::replaceOccurences(countryText, "templateNeuPop", to_string(country.second.attributeDoubles["neutrality"]));
		pU::writeFile(tempPath, countryText);
	}
}

void Hoi4Parser::writeHistoryUnits(std::string path, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: History: Deploying the Troops\n";
	auto content = pU::readFile("resources\\hoi4\\history\\default_unit_template.txt");
	auto unitBlock = pU::readFile("resources\\hoi4\\history\\unit_block.txt");
	auto weakTemplates = pU::readFile("resources\\hoi4\\history\\divisionTemplateWeak.txt");
	auto regionalTemplates = pU::readFile("resources\\hoi4\\history\\divisionTemplateRegional.txt");
	auto majorTemplates = pU::readFile("resources\\hoi4\\history\\divisionTemplateMajor.txt");
	auto IDMapFile = pU::getLines("resources\\hoi4\\history\\divisionIDMapper.txt");
	std::map<int, std::string> IDMap;
	for (auto& line : IDMapFile)
	{
		if (line.size())
		{
			auto lineTokens = pU::getTokens(line, ';');
			IDMap[stoi(lineTokens[0])] = lineTokens[1];
		}
	}
	for (auto country : countries)
	{
		std::string unitFile = content;
		if (country.second.attributeStrings["rank"] == "major")
		{
			ParserUtils::replaceOccurences(unitFile, "templateTemplateBlock", majorTemplates);
		}
		else if (country.second.attributeStrings["rank"] == "regional") {
			ParserUtils::replaceOccurences(unitFile, "templateTemplateBlock", regionalTemplates);
		}
		else
		{
			ParserUtils::replaceOccurences(unitFile, "templateTemplateBlock", weakTemplates);
		}
		std::string totalUnits = "";
		for (int i = 0; i < country.second.attributeVectors["units"].size(); i++)
		{
			for (int x = 0; x < country.second.attributeVectors["units"][i]; x++)
			{
				auto tempUnit = unitBlock;
				ParserUtils::replaceOccurences(tempUnit, "templateDivisionName", IDMap[i]);
				ParserUtils::replaceOccurences(tempUnit, "templateLocation", to_string(country.second.ownedRegions[0].gameProvinces[0].ID + 1));
				totalUnits += tempUnit;
			}
		}


		ParserUtils::replaceOccurences(unitFile, "templateUnitBlock", totalUnits);
		// units
		auto tempPath = path + country.first + "_1936.txt";
		pU::writeFile(tempPath, unitFile);




		// navies
		tempPath = path + country.first + "_1936_naval.txt";
		pU::writeFile(tempPath, "");
		tempPath = path + country.first + "_1936_naval_mtg.txt";
		pU::writeFile(tempPath, "");
	}
}

void Hoi4Parser::dumpCommonCountries(std::string path, std::string hoiPath, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: Common: Writing Countries\n";
	auto content = pU::readFile("resources\\hoi4\\common\\country_default.txt");
	auto colorsTxtTemplate = pU::readFile("resources\\hoi4\\common\\colors.txt");
	std::string colorsTxt = pU::readFile(hoiPath);
	for (auto country : countries)
	{
		auto tempPath = path + country.second.name + ".txt";
		auto countryText = content;
		auto colourString = pU::replaceOccurences(pU::ostreamToString(country.second.colour), ";", " ");
		pU::replaceOccurences(countryText, "templateCulture", country.second.attributeStrings["gfxCulture"]);
		pU::replaceOccurences(countryText, "templateColour", colourString);
		pU::writeFile(tempPath, countryText);
		auto templateCopy = colorsTxtTemplate;
		pU::replaceOccurences(templateCopy, "templateTag", country.first);
		pU::replaceOccurences(templateCopy, "templateColour", colourString);
		colorsTxt.append(templateCopy);
	}
	pU::writeFile(path + "colors.txt", colorsTxt);
}

void Hoi4Parser::dumpCommonCountryTags(std::string path, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: Common: Writing Country Tags\n";
	std::string content = "";
	for (auto country : countries)
	{
		content.append(country.first + " = countries/" + country.second.name + ".txt\n");
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::writeCountryNames(std::string path, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: Localisation: Writing Country Names\n";
	NameGenerator nG;
	std::string content = "l_english:\n";
	vector<std::string> ideologies{ "fascism", "communism", "neutrality", "democratic" };

	for (auto c : countries)
	{
		for (auto& ideology : ideologies)
		{
			auto ideologyName = nG.modifyWithIdeology(ideology, c.second.name, c.second.adjective);
			content += " " + c.first + "_" + ideology + ":0 \"" + ideologyName + "\"\n";
			content += " " + c.first + "_" + ideology + "_DEF:0 \"" + ideologyName + "\"\n";;
			content += " " + c.first + "_" + ideology + "_ADJ:0 \"" + c.second.adjective + "\"\n";;
		}
	}
	pU::writeFile(path + "countries_l_english.yml", content, true);
}

void Hoi4Parser::writeStateNames(std::string path, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: Localisation: Writing State Names\n";
	std::string content = "l_english:\n";

	for (auto c : countries)
	{
		for (auto& region : c.second.ownedRegions)
			content += " STATE_" + to_string(region.ID + 1) + ":0 \"" + region.name + "\"\n";
	}
	pU::writeFile(path + "state_names_l_english.yml", content, true);
}

void Hoi4Parser::writeFoci(std::string path, vector<NationalFocus> foci, const std::map<std::string, Country>& countries)
{
	std::cout << "HOI4 Parser: History: Demanding Danzig\n";
	std::string baseTree = ParserUtils::readFile("resources\\hoi4\\ai\\focusBase.txt");
	std::string attackFocus = ParserUtils::readFile("resources\\hoi4\\ai\\attackFocus.txt");


	for (auto& c : countries)
	{
		std::string treeContent = baseTree;
		std::string tempContent = "";
		std::vector<NationalFocus> countryFoci;
		for (auto& focus : foci)
		{
			if (focus.sourceTag == c.first)
				countryFoci.push_back(focus);
		}
		for (auto& countryFocus : countryFoci)
		{
			if (countryFocus.fType == countryFocus.attack)
			{
				tempContent += attackFocus;
				ParserUtils::replaceOccurences(tempContent, "templateID", to_string(countryFocus.ID));
				ParserUtils::replaceOccurences(tempContent, "templateSourceTag", c.first);
				ParserUtils::replaceOccurences(tempContent, "templateDestTag", countryFocus.destTag);
				ParserUtils::replaceOccurences(tempContent, "templateXPosition", to_string(countryFocus.position[0]));
				ParserUtils::replaceOccurences(tempContent, "templateYPosition", to_string(countryFocus.position[1]));
			}
		}
		ParserUtils::replaceOccurences(treeContent, "templateFocusTree", tempContent);
		ParserUtils::replaceOccurences(treeContent, "templateSourceTag", c.first);
		ParserUtils::writeFile(path + c.second.name + ".txt", treeContent);
	}
}
