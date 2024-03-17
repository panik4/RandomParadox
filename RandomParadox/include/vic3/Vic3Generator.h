#pragma once
#include "Vic3Country.h"
#include "Vic3Region.h"
#include "vic3/Vic3Importer.h"
#include "vic3/Vic3Utils.h"
#include <generic/ScenarioGenerator.h>
#include <iostream>
#include <random>
#include <vector>
#include <cmath>
#include <utility>
#include <memory>
namespace Scenario::Vic3 {

struct Vic3StratRegion {
  std::set<int> areaIDs;
  std::string name;
};

class Generator : public Scenario::Generator {
  std::map<std::string, Technology> techs;
  std::map<std::string, TechnologyLevel> techLevels;
  std::map<std::string, ProductionmethodGroup> productionmethodGroups;
  std::map<std::string, Productionmethod> productionmethods;
  std::vector<BuildingType> buildingsTypes;
  std::map<std::string, Good> goods;
  std::map<std::string, Buypackage> buypackages;
  std::map<std::string, PopNeed> popNeeds;
  // to search for productionmethods that produce this good
  std::map<std::string, std::vector<Productionmethod>> goodToProdMethodsOutput;
  // to search for productionmethods that require this good
  std::map<std::string, std::vector<Productionmethod>> goodToProdMethodsInput;

  // to search for buildings that use this productionMethod
  std::map<std::string, std::vector<BuildingType>> productionMethodToBuildingTypes;
  // to search for buildings that produce this good
  std::map<std::string, std::vector<BuildingType>> goodToBuildingTypes;

public:
  std::vector<std::shared_ptr<Region>> vic3Regions;
  std::map<std::string, std::shared_ptr<Country>> vic3Countries;
  Generator();
  Generator(const std::string &configSubFolder);

  virtual void mapRegions();
  void distributePops();
  void totalArableLand(const std::vector<float> &arableLand);
  void totalResourceVal(const std::vector<double> &resPrev,
                        double resourceModifier, const ResConfig& resourceConfig);
  void distributeResources();
  // initialize states
  virtual void initializeStates();
  // map scenario countries to vic3 countries
  virtual void mapCountries();
  // initialize countries
  virtual void initializeCountries();
  // read buildings, techs, etc from the game files
  void importData(const std::string &path);

  // figure out how countries stand towards each other - colonies,
  // protectorates, vassals, allies, customs union etc
  void diplomaticRelations();

  // combine countries into markets according to their diplomatic stances
  void createMarkets();

  void
  calculateNeeds();

  void distributeBuildings();
};
} // namespace Scenario::Vic3