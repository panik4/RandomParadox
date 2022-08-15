#pragma once
#include "generic/ParserUtils.h"
#include "utils/Bitmap.h"
#include "utils/Logging.h"
#include <iostream>
namespace Scenario::Hoi4MapPainting {
struct Region {
  std::string owner;
  std::vector<std::string> coreHolders;
  std::vector<int> provinceIDs;
};
namespace Detail {
Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path);
std::vector<Region> readStates(const std::string &path);
std::vector<Fwg::Province> readProvinceMap(const std::string &path);
void stateBitmap(const std::string &inPath, Fwg::Gfx::Bitmap countries,
                 const std::vector<Fwg::Province> &provinces,
                 const std::vector<Region> &states);
} // namespace Detail
void output(const std::string &inPath, const std::string &outputPath,
            bool multiCore, bool exportMap, const std::string &inputMap);
}; // namespace Scenario::Hoi4MapPainting
