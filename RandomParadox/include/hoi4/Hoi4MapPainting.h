#pragma once
#include "generic/ParserUtils.h"
#include "utils/Bitmap.h"
#include <iostream>
namespace Scenario::Hoi4MapPainting {

namespace Detail {
Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path);
std::vector<std::vector<int>> readStates(const std::string &path);
std::vector<Fwg::Province> readProvinceMap(const std::string &path);
} // namespace Detail

void output(const std::string &inPath, const std::string &outputPath,
            bool multiCore);
}; // namespace Scenario::Hoi4MapPainting
