#pragma once
#include "Hoi4Generator.h"
#include "Hoi4Parsing.h"
#include "generic/ParserUtils.h"
#include "utils/Bitmap.h"
#include "utils/Logging.h"
#include <iostream>
#include <tuple>
namespace Scenario::Hoi4::MapPainting {

struct ChangeHolder {
  std::set<int> deletedProvs;
  std::set<int> changedProvs;
  std::set<int> newProvs;
  // transformations of province IDs in case of deletions
  std::map<int, int> provIdMapping;

  std::set<int> deletedStates;
  std::set<int> changedStates;
  std::map<int, int> stateIdMapping;
  // tracks old and new state a province is assigned to
  std::map<std::shared_ptr<GameProvince>,
           std::vector<std::shared_ptr<Scenario::Region>>>
      stateChanges;

  std::set<std::string> deletedCountries;
  std::set<std::string> changedCountries;
  // tracks old and new owner of a state
  std::map<std::shared_ptr<Scenario::Region>, std::array<std::string, 2>>
      ownerChanges;
};

namespace Detail {

} // namespace Detail
namespace Countries {
void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes);

}

namespace States {
void updateStates(Generator &hoi4Gen, ChangeHolder &changes);
void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes);
// void stateBitmap(const std::string &inPath, Fwg::Gfx::Bitmap countries,
//                  const std::vector<Fwg::Province> &provinces,
//                  const std::vector<Region> &states);
} // namespace States

namespace Provinces {
void edit(const std::string &inPath, const std::string &outputPath,
          const std::string &mapName, Generator &hoi4Gen,
          ChangeHolder &changes);
}

}; // namespace Scenario::Hoi4::MapPainting
