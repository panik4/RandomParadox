#pragma once
#include <string>
#include "ParserUtils.h"
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"

class ResourceLoader
{
public:
	ResourceLoader();
	~ResourceLoader();
	Bitmap loadProvinceMap(std::string gamePath);
	Bitmap loadHeightMap(std::string gamePath);
	std::vector<std::string> loadStates(std::string gamePath);
	std::vector<std::string> loadDefinition(std::string gamePath);
	std::vector<std::string> loadForbiddenTags(std::string gamePath);
};

