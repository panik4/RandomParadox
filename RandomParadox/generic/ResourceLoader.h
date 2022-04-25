#pragma once
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"
#include "ParserUtils.h"
#include <string>

class ResourceLoader {
public:
  ResourceLoader();
  ~ResourceLoader();
  Bitmap loadProvinceMap(std::string gamePath);
  Bitmap loadHeightMap(std::string gamePath);
  std::vector<std::string> loadStates(std::string gamePath);
  std::vector<std::string> loadDefinition(std::string gamePath);
  std::vector<std::string> loadForbiddenTags(std::string gamePath);
};
