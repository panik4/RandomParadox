#pragma once
#include <string>
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "ParserUtils.h"

class ResourceLoader
{
public:
	ResourceLoader();
	~ResourceLoader();
	void loadBitmaps();
	
	// hoi4
	void loadHistory(std::string gamePath);
	void loadProvinces(std::string gamePath);
	Bitmap loadProvinceMap(std::string gamePath);
	Bitmap loadHeightMap(std::string gamePath);
	vector<std::string> loadStates(std::string gamePath);
	vector<std::string> loadDefinition(std::string gamePath);
	vector<std::string> loadForbiddenTags(std::string gamePath);
	//vector<std::string> loadStrategicRegions();
};

