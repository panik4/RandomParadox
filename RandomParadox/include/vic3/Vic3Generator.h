#pragma once
#include <generic/ScenarioGenerator.h>
#include "Vic3Region.h"
#include "Vic3Country.h"

namespace Scenario::Vic3 {

struct Vic3StratRegion {
  std::set<int> areaIDs;
  std::string name;
};

class Generator : public Scenario::Generator {
 // std::vector<Vic3StratRegion> vic3StratRegions;

public:
  std::vector<std::shared_ptr<Region>> vic3Regions;
  Generator();
  Generator(const std::string &configSubFolder);
  // initialize states
  virtual void mapRegions();
  // initialize states
  virtual void initializeStates();
  // initialize countries
  virtual void initializeCountries();
  //void generateRegions(std::vector<std::shared_ptr<Region>> &regions);
  void distributePops();
 // std::vector<Vic3StratRegion> getVic3Regions() { return vic3StratRegions; };
};
} // namespace Scenario::Vic3