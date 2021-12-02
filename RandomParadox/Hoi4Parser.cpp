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
		content.append(to_string(region.ID));
		content.append("={");
		content.append(to_string(region.provinces[0]->provID + 1));
		content.append(" }\n");
	}
	writeFile(path, content);
}

// places building positions
void Hoi4Parser::dumpBuildings(std::string path, vector<Region> regions)
{
	vector<std::string> buildingTypes{ "arms_factory", "industrial_complex", "air_base",
		"naval_base", "bunker", "coastal_bunker", "dockyard", "anti_air_building",
		"synthetic_refinery", "nuclear_reactor", "rocket_site", "radar_station" };
	std::string content;
	auto random = Data::getInstance().random2;
	// stateId; type; pixelX, rotation??, pixelY, rotation??, 0??}
	// 1; arms_factory; 2946.00; 11.63; 1364.00; 0.45; 0
	for (auto region : regions)
	{
		for (auto type : buildingTypes)
		{
			//auto cityPix = *select_random(cityPixels, r);
			auto prov = *select_random(region.provinces);
			auto pix = *select_random(prov->pixels);
			auto widthPos = pix % Data::getInstance().width;
			auto heightPos = pix / Data::getInstance().width;
			std::vector<std::string> arguments{ to_string(region.ID + 1), type, to_string(widthPos), to_string(0.5), to_string(heightPos), to_string(0.5), "0" };
			content.append(csvFormat(arguments, ';', false));
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
		auto seaType = prov->isLake || prov->sea ? "sea" : "land";
		auto coastal = prov->coastal ? "true" : "false";
		std::string terraintype;
		if (prov->sea)
			terraintype = "sea";
		if (prov->isLake)
			terraintype = "lake";
		else
			terraintype = "plains";
		std::vector<std::string> arguments{ to_string(prov->provID),
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
		content.append(to_string(region.ID));
		content.append("={");
		content.append(to_string(region.provinces[0]->provID + 1));
		content.append(" }\n");
	}
	writeFile(path, content);
}

void Hoi4Parser::dumpStrategicRegions(std::string path, vector<Region> regions)
{
	// regionId={provId }
	for (auto region : regions)
	{
		std::string content;
		content.append(to_string(region.ID));
		content.append("={");
		content.append(to_string(region.provinces[0]->provID + 1));
		content.append(" }\n");
		writeFile(path, content);
	}
}
