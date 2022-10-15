#pragma once
#include "FastWorldGenerator.h"
#include "Flag.h"
#include "GameProvince.h"
#include "GameRegion.h"
#include "NameGenerator.h"
#include "PdoxContinent.h"
#include "PdoxCountry.h"
#include "ResourceLoading.h"
#include "RandNum/RandNum.h"
#include <map>
namespace Scenario {
class Generator {
  // containers
  std::vector<Fwg::Region> baseRegions;
  std::map<std::string, Fwg::Gfx::Bitmap> bitmaps;

public:
  Generator();
  // vars - used for every game
  NameGeneration::NameData nData;
  Fwg::FastWorldGenerator fwg;
  int numCountries;
  // containers - used for every game
  std::vector<Fwg::Province *> provinces;
  std::vector<PdoxContinent> pdoxContinents;
  Fwg::Utils::ColourTMap<std::shared_ptr<Region>> stateColours;
  std::vector<std::shared_ptr<Region>> gameRegions;
  std::vector<std::shared_ptr<GameProvince>> gameProvinces;
  std::set<std::string> tags;
  Fwg::Utils::ColourTMap<std::string> colourMap;
  std::map<std::string, PdoxCountry> countries;
  // constructors/destructors
  Generator(Fwg::FastWorldGenerator &fwg);
  ~Generator();
  /* member functions*/
  // print a map showing all countries for debug purposes
  Fwg::Gfx::Bitmap dumpDebugCountrymap(const std::string &path);
  // specific preparations. Used by each game, BUT to create game scenario
  void loadRequiredResources(const std::string &gamePath);
  // generic preparations. However, if desired, there are necessary preparations
  // for every game such as reading in the existing worldmap, states, regions,
  // provinces etc
  void generateWorld();
  // map base continents to generic paradox compatible game continents
  void mapContinents();
  // map base regions to generic paradox compatible game regions
  void mapRegions();
  // map base provinces to generic game regions
  void mapProvinces();
  // calculating populations in states
  void generatePopulations();
  // determine development from habitability, population density and randomness
  void generateDevelopment();
  // mapping terrain types of FastWorldGen to paradox compatible terrains
  void mapTerrain();
  // GameRegions are used for every single game,
  std::shared_ptr<Region> &findStartRegion();
  // and countries are always created the same way
  void generateCountries(int numCountries, const std::string &gamePath);
  // see which country neighbours which
  void evaluateNeighbours();
};
} // namespace Scenario
