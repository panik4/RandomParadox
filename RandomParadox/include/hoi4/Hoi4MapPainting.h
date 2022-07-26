#pragma once
#include <iostream>
#include "utils/Bitmap.h"
#include "Hoi4Parsing.h"
namespace Scenario::Hoi4MapPainting {

// all maps
Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path);

// default map
std::vector<std::vector<int>> readStates(const std::string &path);

std::vector<Fwg::Province> readProvinceMap(const std::string &path);

void output(std::vector<std::vector<int>> states,
            std::vector<Fwg::Province> provinces, const std::string &inPath,
            const std::string &outputPath);

// custom map

}; // namespace Scenario::Hoi4MapPainting
