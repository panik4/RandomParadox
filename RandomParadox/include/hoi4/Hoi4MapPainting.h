#pragma once
#include "Hoi4Generator.h"
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

void stateBitmap(const std::string &inPath, Fwg::Gfx::Bitmap countries,
                 const std::vector<Fwg::Province> &provinces,
                 const std::vector<Region> &states);
} // namespace Detail
void output(const std::string &inPath, const std::string &outputPath,
            bool multiCore, bool exportMap, const std::string &inputMap);
namespace ProvinceEditing {
void provinceEditing(const std::string &inPath, const std::string &outputPath,
                     const std::string &mapName, const Generator &hoi4Gen);
}

}; // namespace Scenario::Hoi4MapPainting
