#pragma once
#include "FastWorldGenerator.h"
#include "parsing/ParserUtils.h"
#include <string>

namespace Rpx::IO {
Fwg::Gfx::Image loadProvinceMap(const std::string &gamePath);
Fwg::Gfx::Image loadHeightMap(const std::string &gamePath);
std::vector<std::string> loadStates(const std::string &gamePath);
std::vector<std::string> loadDefinition(const std::string &gamePath);
std::vector<std::string> loadForbiddenTags(const std::string &gamePath);
}; // namespace IO
