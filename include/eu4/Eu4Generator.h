#pragma once
#include <generic/ModGenerator.h>

namespace Rpx::Eu4 {
// eu4 regions consist of multiple areas, which are collections of provinces
struct eu4Region {
  std::set<int> areaIDs;
  std::string name;
};

class Generator : public Rpx::ModGenerator {
  std::vector<eu4Region> eu4regions;

public:
  Generator();
  Generator(const std::string &configSubFolder);
  void generateRegions(std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions);

  // initialize countries
  virtual void mapCountries();
  std::vector<eu4Region> getEu4Regions() { return eu4regions; };
};
} // namespace Rpx::Eu4