#pragma once
#include <string>
#include "ParserUtils.h"
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"

class ResourceLoader
{
public:
	ResourceLoader();
	~ResourceLoader();
	// member functions
	void loadBitmaps();	
	//  member functions - hoi4
	void loadHistory(std::string gamePath);
	void loadProvinces(std::string gamePath);
	Bitmap loadProvinceMap(std::string gamePath);
	Bitmap loadHeightMap(std::string gamePath);
	std::vector<std::string> loadStates(std::string gamePath);
	std::vector<std::string> loadDefinition(std::string gamePath);
	std::vector<std::string> loadForbiddenTags(std::string gamePath);
};

