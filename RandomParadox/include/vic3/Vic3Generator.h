#pragma once
#include "Vic3Country.h"
#include "Vic3Region.h"
#include "vic3/Vic3Utils.h"
#include <generic/ScenarioGenerator.h>

namespace Scenario::Vic3 {

struct Vic3StratRegion {
  std::set<int> areaIDs;
  std::string name;
};

class Generator : public Scenario::Generator {

public:
  std::vector<std::shared_ptr<Region>> vic3Regions;
  std::map<std::string, std::shared_ptr<Country>> vic3Countries;
  Generator();
  Generator(const std::string &configSubFolder);

  virtual void mapRegions();
  void distributePops();
  void totalArableLand(const std::vector<float> &arableLand);
  void totalResourceVal(const std::vector<double> &resPrev,
                        double resourceModifier, ResourceType type);
  void distributeResources();
  // initialize states
  virtual void initializeStates();
  // map scenario countries to vic3 countries
  virtual void mapCountries();
  // initialize countries
  virtual void initializeCountries();
};
} // namespace Scenario::Vic3