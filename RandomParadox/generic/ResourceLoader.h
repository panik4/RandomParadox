#pragma once
#include <string>
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "ParserUtils.h"

class ResourceLoader
{
	std::string hoi4Path;
	bool genHoi4;
public:
	ResourceLoader();
	ResourceLoader(bool genHoi4, std::string hoi4Path);
	~ResourceLoader();
	void loadBitmaps();
	
	// hoi4
	void loadHistory();
	void loadProvinces();
	Bitmap loadProvinceMap();
	Bitmap loadHeightMap();
	vector<std::string> loadStates();
	vector<std::string> loadDefinition();
	//vector<std::string> loadStrategicRegions();
};

