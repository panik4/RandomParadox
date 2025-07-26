#pragma once
#include "FastWorldGenerator.h"
#include "parsing/ParserUtils.h"
#include <string>

namespace Rpx::ResourceLoading {
Fwg::Gfx::Bitmap loadProvinceMap(const std::string &gamePath);
Fwg::Gfx::Bitmap loadHeightMap(const std::string &gamePath);
std::vector<std::string> loadStates(const std::string &gamePath);
std::vector<std::string> loadDefinition(const std::string &gamePath);
std::vector<std::string> loadForbiddenTags(const std::string &gamePath);
}; // namespace ResourceLoading
