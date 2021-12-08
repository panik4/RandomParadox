#include "Hoi4Parser.h"



Hoi4Parser::Hoi4Parser()
{
}


Hoi4Parser::~Hoi4Parser()
{
}

void Hoi4Parser::writeFile(std::string path, std::string content)
{
	ofstream myfile;
	myfile.open(path);
	myfile << content;
	myfile.close();
}

std::string Hoi4Parser::readFile(std::string path)
{
	std::string content;
	std::string line;
	ifstream myfile;
	myfile.open(path);
	while (getline(myfile, line))
	{
		content.append(line + "\n");
	}
	myfile.close();
	return content;
}

std::string Hoi4Parser::csvFormat(vector<std::string> arguments, char delimiter, bool trailing)
{
	vector<string>::iterator arg;
	std::string retString("");
	for (arg = arguments.begin(); arg != arguments.end(); arg++)
	{
		retString.append(*arg);
		if (!trailing && arguments.end() - arg == 1)
		{
			continue;
		}
		retString.append(string{ delimiter });

	}
	retString.append("\n");
	return retString;
}

void Hoi4Parser::replaceOccurences(std::string& content, std::string key, std::string value)
{
	auto pos = 0;
	do
	{
		pos = content.find(key);
		if (pos != string::npos)
		{
			content.replace(pos, key.length(), value);
		}
	} while (pos != string::npos);
}


void Hoi4Parser::replaceLine(std::string& content, std::string key, std::string value)
{
	auto pos = 0;
	pos = content.find(key);
	if (pos != string::npos)
	{
		auto lineEnd = content.find("\n", pos);
		content.replace(pos, lineEnd - pos, value);
	}
}

void Hoi4Parser::dumpAdj(std::string path)
{
	// From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
	// empty file for now
	std::string content;
	content.append("From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment");
	writeFile(path, content);
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
	writeFile(path, content);
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

	auto widthPos = pix % Data::getInstance().width;
	auto heightPos = pix / Data::getInstance().width;
	std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string(0.5), to_string(heightPos), to_string(0.5), "0" };
	return csvFormat(arguments, ';', false);
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
						auto widthPos = pix % Data::getInstance().width;
						auto heightPos = pix / Data::getInstance().width;
						std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string(0.5), to_string(heightPos), to_string(0.5), "0" };
						content.append(csvFormat(arguments, ';', false));
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
						auto widthPos = pix % Data::getInstance().width;
						auto heightPos = pix / Data::getInstance().width;
						std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string(0.5), to_string(heightPos), to_string(0.5), "0" };
						content.append(csvFormat(arguments, ';', false));
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
	writeFile(path, content);
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
	writeFile(path, content);
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
		std::string terraintype;
		if (prov->sea)
			terraintype = "sea";
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
			to_string(prov->sea ? 0 : prov->continentID + 1) // 0 is for sea, no continent
		};
		content.append(csvFormat(arguments, ';', false));
	}
	writeFile(path, content);
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
	writeFile(path, content);
}

void Hoi4Parser::dumpUnitStacks(std::string path, vector<Province*> provinces)
{
	// 1;0;3359.00;9.50;1166.00;0.00;0.08
	// provID, neighbour?, xPos, zPos yPos, rotation(3=north, 0=south, 1.5=east,4,5=west), ??
	// provID, xPos, ~10, yPos, ~0, 0,5
	std::cout << atan2(-4, 0) << std::endl;;
	std::cout << getAngleB(768, 256, 512) << std::endl;;
	std::cout << atan2(2, 0) << std::endl;;
	std::cout << getAngleB(256, 768, 512) << std::endl;;
	std::cout << atan2(0, 2) << std::endl;;
	std::cout << getAngleB(256, 511, 512) << std::endl;;
	std::cout << atan2(0, -2) << std::endl;;
	std::cout << getAngleB(256, 512, 512) << std::endl;;
	std::cout << atan2(-2, -2) << std::endl;;
	std::cout << getAngleB(768, 255, 512) << std::endl;;
	//std::cout << atan2(2, 2) << std::endl;;
	std::cout << atan2(-2, 2) << std::endl;;
	std::cout << getAngleB(768, 257, 512) << std::endl;;
	//std::cout << atan2(2, -2) << std::endl;;


	//std::cout << getAngleB(256, 1024, 512) << std::endl;;
	//std::cout << getAngleB(256, 0, 512) << std::endl;;
	//std::cout << getAngleB(256, 511, 512) << std::endl;;
	// for each neighbour add move state in the direction of the neighbour. 0 might be stand still
	std::string content{ "" };
	for (auto prov : provinces)
	{
		int position = 0;
		auto pix = *select_random(prov->pixels);
		auto widthPos = pix % Data::getInstance().width;
		auto heightPos = pix / Data::getInstance().width;
		std::vector<std::string> arguments{ to_string(prov->provID + 1), to_string(position), to_string(widthPos), to_string(1), to_string(heightPos), to_string(0.0), "0.0" };
		content.append(csvFormat(arguments, ';', false));
		for (auto neighbour : prov->adjProv)
		{
			position++;
			double angle;
			auto nextPos = prov->getPositionBetweenProvinces(*neighbour, Data::getInstance().width, angle);
			angle += 1.57;
			//auto angle = 1.57 + getAngleB(pix, nextPos, Data::getInstance().width);
			//std::cout << angle << std::endl;
			auto widthPos = nextPos % Data::getInstance().width;
			auto heightPos = nextPos / Data::getInstance().width;
			std::vector<std::string> arguments{ to_string(prov->provID + 1), to_string(position), to_string(widthPos), to_string(15), to_string(heightPos), to_string(angle), "0.0" };
			content.append(csvFormat(arguments, ';', false));
		}
	}
	writeFile(path, content);
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
		content.append(csvFormat(arguments, ';', false));
	}
	writeFile(path, content);
}

void Hoi4Parser::dumpAdjacencyRules(std::string path)
{
	std::string content{ "" };
	// empty for now
	writeFile(path, content);
}

// awful, just awful
void Hoi4Parser::dumpStrategicRegions(std::string path, vector<Region> regions)
{
	auto templateContent = readFile("resources\\hoi4\\strategic_region.txt");

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
		replaceOccurences(content, "templateID", to_string(region.ID + 1));
		replaceOccurences(content, "template_provinces", provString);
		writeFile(path + "\\" + to_string(region.ID + 1) + ".txt", content);
	}
}

void Hoi4Parser::dumpSupplyAreas(std::string path, vector<Region> regions)
{
	auto templateContent = readFile("resources\\hoi4\\SupplyArea.txt");
	for (auto region : regions)
	{
		auto content = templateContent;
		replaceOccurences(content, "templateID", to_string(region.ID + 1));
		replaceOccurences(content, "templateState", to_string(region.ID + 1));
		writeFile(path + "\\" + to_string(region.ID + 1) + "-SupplyArea.txt", content);
	}
}

void Hoi4Parser::dumpStates(std::string path, vector<Region> regions)
{
	auto templateContent = readFile("resources\\hoi4\\state.txt");
	for (auto region : regions)
	{
		if (region.sea)
			continue;
		sort(region.provinces.begin(), region.provinces.end());
		region.provinces.erase(unique(region.provinces.begin(), region.provinces.end()), region.provinces.end());
		std::string provString{ "" };
		for (auto prov : region.provinces)
		{
			provString.append(to_string(prov->provID + 1));
			provString.append(" ");
		}
		auto content = templateContent;
		replaceOccurences(content, "templateID", to_string(region.ID + 1));
		replaceOccurences(content, "template_provinces", provString);
		writeFile(path + "\\" + to_string(region.ID + 1) + ".txt", content);
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
		//auto content = readFile(dir_entry.path.);
		std::stringstream pathStream;
		pathStream << dir_entry.path();
		std::string pathString;
		pathString = pathStream.str();
		std::cout << pathString << '\n';
		std::string filename = pathString.substr(pathString.find_last_of("\\") + 1, pathString.back() - pathString.find_last_of("\\"));
		std::cout << filename << '\n';
		auto content = readFile(pathString);
		if (content.find("SOV") != -1)
			cout << "SA";
		replaceLine(content, "capital =", "capital = " + to_string(ids[random() % ids.size()]));
		writeFile(path + filename, content);
	}
}
