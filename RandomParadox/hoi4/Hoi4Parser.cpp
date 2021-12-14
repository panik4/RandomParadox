#include "Hoi4Parser.h"



Hoi4Parser::Hoi4Parser()
{
}


Hoi4Parser::~Hoi4Parser()
{
}
void Hoi4Parser::dumpAdj(std::string path)
{
	// From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
	// empty file for now
	std::string content;
	content.append("From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment");
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpAirports(std::string path, vector<Region> regions)
{
	std::string content;
	// regionId={provId }
	for (auto region : regions)
	{
		if (region.sea)
			continue;
		content.append(to_string(region.ID + 1));
		content.append("={");
		content.append(to_string(region.provinces[0]->provID + 1));
		content.append(" }\n");
	}
	pU::writeFile(path, content);
}

std::string Hoi4Parser::getBuildingLine(std::string type, Region& region, bool coastal)
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
	std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string(9.5), to_string(heightPos), to_string((float)-1.57), "0" };
	return pU::csvFormat(arguments, ';', false);
}

// places building positions
void Hoi4Parser::dumpBuildings(std::string path, vector<Region> regions)
{
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
					content.append(getBuildingLine(type, region, false));
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
						std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string(9.5), to_string(heightPos), to_string(0.5), "0" };
						content.append(pU::csvFormat(arguments, ';', false));
					}
				}
			}
			else if (type == "anti_air_building")
			{
				for (int i = 0; i < 3; i++)
				{
					content.append(getBuildingLine(type, region, false));
				}
			}
			else if (type == "coastal_bunker" || type == "naval_base")
			{
				for (auto prov : region.provinces)
				{
					if (prov->coastal)
					{
						auto pix = *select_random(prov->coastalPixels);
						uint32_t provID = 0;
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
											provID = neighbour->provID;
										}
									}
								}
							}
						}
						auto widthPos = (pix % Data::getInstance().width);
						auto heightPos = /*Data::getInstance().height -*/ (pix / Data::getInstance().width);
						std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string(9.5), to_string(heightPos), to_string(0.5), to_string(provID) };
						content.append(pU::csvFormat(arguments, ';', false));
					}
				}
			}
			else if (type == "dockyard")
			{
				if (coastal)
				{
					content.append(getBuildingLine(type, region, coastal));
				}
			}
			else {
				{
					content.append(getBuildingLine(type, region, false));
				}
			}
		}
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpContinents(std::string path, vector<Continent> continents)
{
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

void Hoi4Parser::dumpDefinition(std::string path, vector<Province*> provinces)
{
	// province id; r value; g value; b value; province type (land/sea/lake); coastal (true/false); terrain (plains/hills/urban/etc. Defined for land or sea provinces in common/terrain); continent (int)
	// 0;0;0;0;land;false;unknown;0

	// terraintypes: ocean, lakes, forest, hills, mountain, plains, urban, jungle, marsh, desert, water_fjords, water_shallow_sea, water_deep_ocean
	// TO DO: properly map terrain types from climate

	std::string content{ "0;0;0;0;land;false;unknown;0\n" };
	for (auto prov : provinces)
	{
		auto seaType = prov->sea ? "sea" : "land";
		auto coastal = prov->coastal ? "true" : "false";
		if (prov->sea)
		{
			for (auto prov2 : prov->adjProv)
			{
				if (!prov2->sea)
					coastal = "true";
			}
		}
		std::string terraintype;
		if (prov->sea)
			terraintype = "ocean";
		else
			terraintype = "plains";
		if (prov->isLake)
		{
			terraintype = "lakes";
			seaType = "lake";
		}
		std::vector<std::string> arguments{ to_string(prov->provID + 1),
			to_string(prov->colour.getRed()),
			to_string(prov->colour.getGreen()),
			to_string(prov->colour.getBlue()),
			seaType, coastal, terraintype,
			to_string(prov->sea || prov->isLake ? 0 : prov->continentID + 1) // 0 is for sea, no continent
		};
		content.append(pU::csvFormat(arguments, ';', false));
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpRocketSites(std::string path, vector<Region> regions)
{
	std::string content;
	// regionId={provId }
	for (auto region : regions)
	{
		if (region.sea)
			continue;
		content.append(to_string(region.ID + 1));
		content.append("={");
		content.append(to_string(region.provinces[0]->provID + 1));
		content.append(" }\n");
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpUnitStacks(std::string path, vector<Province*> provinces)
{
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
		std::vector<std::string> arguments{ to_string(prov->provID + 1), to_string(position), to_string(widthPos), to_string(1), to_string(heightPos), to_string(0.0), "0.0" };
		content.append(pU::csvFormat(arguments, ';', false));
		for (auto neighbour : prov->adjProv)
		{
			position++;
			double angle;
			auto nextPos = prov->getPositionBetweenProvinces(*neighbour, Data::getInstance().width, angle);
			angle += 1.57;
			auto widthPos = nextPos % Data::getInstance().width;
			auto heightPos = nextPos / Data::getInstance().width;
			std::vector<std::string> arguments{ to_string(prov->provID + 1), to_string(position), to_string(widthPos), to_string(15), to_string(heightPos), to_string(angle), "0.0" };
			content.append(pU::csvFormat(arguments, ';', false));
		}
	}
	pU::writeFile(path, content);
}

void Hoi4Parser::dumpWeatherPositions(std::string path, vector<Region> regions)
{
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
	std::string content{ "" };
	// empty for now
	pU::writeFile(path, content);
}

// awful, just awful
void Hoi4Parser::dumpStrategicRegions(std::string path, vector<Region> regions)
{
	auto templateContent = pU::readFile("resources\\hoi4\\strategic_region.txt");

	for (auto region : regions)
	{
		sort(region.provinces.begin(), region.provinces.end());
		region.provinces.erase(unique(region.provinces.begin(), region.provinces.end()), region.provinces.end());
		std::string provString{ "" };
		for (auto prov : region.provinces)
		{
			provString.append(to_string(prov->provID + 1));
			provString.append(" ");
		}
		auto content = templateContent;
		pU::replaceOccurences(content, "templateID", to_string(region.ID + 1));
		pU::replaceOccurences(content, "template_provinces", provString);
		pU::writeFile(path + "\\" + to_string(region.ID + 1) + ".txt", content);
	}
}

void Hoi4Parser::dumpSupplyAreas(std::string path, vector<Region> regions)
{
	auto templateContent = pU::readFile("resources\\hoi4\\SupplyArea.txt");
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

void Hoi4Parser::dumpStates(std::string path, std::map<std::string, Country> countries)
{
	auto templateContent = pU::readFile("resources\\hoi4\\state.txt");
	for (auto country : countries)
	{
		for (auto region : country.second.ownedRegions)
		{
			auto baseRegion = region.baseRegion;
			if (baseRegion.sea)
				continue;
			sort(baseRegion.provinces.begin(), baseRegion.provinces.end());
			baseRegion.provinces.erase(unique(baseRegion.provinces.begin(), baseRegion.provinces.end()), baseRegion.provinces.end());
			std::string provString{ "" };
			for (auto prov : baseRegion.provinces)
			{
				provString.append(to_string(prov->provID + 1));
				provString.append(" ");
			}
			auto content = templateContent;
			pU::replaceOccurences(content, "templateID", to_string(baseRegion.ID + 1));
			pU::replaceOccurences(content, "template_provinces", provString);
			pU::replaceOccurences(content, "templateOwner", "BRA"/*country.first*/);
			pU::writeFile(path + "\\" + to_string(baseRegion.ID + 1) + ".txt", content);
		}
	}
	//for (auto region : regions)
	//{
	//	if (region.sea)
	//		continue;
	//	sort(region.provinces.begin(), region.provinces.end());
	//	region.provinces.erase(unique(region.provinces.begin(), region.provinces.end()), region.provinces.end());
	//	std::string provString{ "" };
	//	for (auto prov : region.provinces)
	//	{
	//		provString.append(to_string(prov->provID + 1));
	//		provString.append(" ");
	//	}
	//	auto content = templateContent;
	//	replaceOccurences(content, "templateID", to_string(region.ID + 1));
	//	replaceOccurences(content, "template_provinces", provString);
	//	writeFile(path + "\\" + to_string(region.ID + 1) + ".txt", content);
	//}
}
void Hoi4Parser::dumpFlags(std::string path, std::map<std::string, Country> countries)
{
	for (auto country : countries)
	{
		
		TextureWriter::writeTGA(country.second.flag.width, country.second.flag.height, country.second.flag.getFlag(), path + country.first + ".tga");
		TextureWriter::writeTGA(country.second.flag.width/2, country.second.flag.height / 2, country.second.flag.resize(country.second.flag.width / 2, country.second.flag.height / 2), path + "\\medium\\" + country.first + ".tga");
		TextureWriter::writeTGA(country.second.flag.width / 8, country.second.flag.height / 8, country.second.flag.resize(country.second.flag.width / 8, country.second.flag.height / 8), path + "\\small\\" + country.first + ".tga");
	}
}
// copy relevant default text files from Hoi4 sources
void Hoi4Parser::copyDefaultOverwrites(std::string pathToHoi4)
{
	// common/abilities/generic_leader_abilities.txt
	// common/ai_equipment/generic_naval.txt
	// common/ai_focuses/generic.txt
	// common/ai_strategy/generic_operation_strats.txt
	// common/ai_templates/generic.txt
	// common/national_focus/generic.txt
	// common/on_actions/generic.txt
	//common/operations/00_operations.txt
	// ....
}

void Hoi4Parser::readDefaultCountries(std::string path, std::string hoiPath)
{
	//const std::experimental::filesystem::path hoiDir{ hoiPath + "\\common\\country_tags\\00_countries.txt"};
	auto lines  = pU::getLines(hoiPath + "\\common\\country_tags\\00_countries.txt");
	for (auto line : lines)
	{
		auto tag = line.substr(0, 3);
		std::cout << tag << std::endl;
		defaultTags.push_back(tag);
	}
}

void Hoi4Parser::writeCompatibilityHistory(std::string path, std::string hoiPath, vector<Region> regions)
{
	vector<int> ids;
	for (auto region : regions)
		if (!region.sea)
			ids.push_back(region.ID);

	vector<std::string> tags{ "BRA" };
	const std::experimental::filesystem::path hoiDir{ hoiPath };
	const std::experimental::filesystem::path modDir{ path };
	auto random = Data::getInstance().random2;
	for (auto const& dir_entry : std::experimental::filesystem::directory_iterator{ hoiDir })
	{
		std::stringstream pathStream;
		pathStream << dir_entry.path();
		std::string pathString;
		pathString = pathStream.str();
		std::cout << pathString << '\n';
		std::string filename = pathString.substr(pathString.find_last_of("\\") + 1, pathString.back() - pathString.find_last_of("\\"));
		std::cout << filename << '\n';
		auto content = pU::readFile(pathString);
		pU::replaceLine(content, "capital =", "capital = " + to_string(1/*+ids[random() % ids.size()]*/));
		pU::writeFile(path + filename, content);
	}
}
