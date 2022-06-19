#pragma once
#include "FastWorldGenerator.h"
#include "ParserUtils.h"
#include <string>

namespace ResourceLoading {
FastWorldGen::Bitmap loadProvinceMap(const std::string &gamePath);
FastWorldGen::Bitmap loadHeightMap(const std::string &gamePath);
std::vector<std::string> loadStates(const std::string &gamePath);
std::vector<std::string> loadDefinition(const std::string &gamePath);
std::vector<std::string> loadForbiddenTags(const std::string &gamePath);
}; // namespace ResourceLoading
