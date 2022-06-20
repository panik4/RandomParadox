#pragma once
#include "FastWorldGenerator.h"
#include "PdoxCountry.h"
#include "Flag.h"
#include "PdoxContinent.h"
#include "GameProvince.h"
#include "GameRegion.h"
#include "NameGenerator.h"
#include "ResourceLoading.h"
#include <map>
namespace Scenario {
class Generator {
  // containers
  std::vector<Fwg::Region> baseRegions;
  std::map<std::string, Fwg::Gfx::Bitmap> bitmaps;
  std::map<std::string, std::string> gamePaths;

public:
  // vars - used for every game
  NameGenerator nG;
  Fwg::FastWorldGenerator fwg;
  int numCountries;
  // containers - used for every game
  std::vector<Fwg::Province *> provinces;
  std::vector<PdoxContinent> pdoxContinents;
  std::vector<Region> gameRegions;
  std::vector<GameProvince> gameProvinces;
  std::set<std::string> tags;
  std::map<std::string, PdoxCountry> countries;
  // constructors/destructors
  Generator(Fwg::FastWorldGenerator &fwg);
  ~Generator();
  /* member functions*/
  // print a map showing all countries for debug purposes
  void dumpDebugCountrymap(std::string path);
  // specific preparations. Used by each game, BUT to create game scenario
  void loadRequiredResources(std::string gamePath);
  // game specific preparations. Only used for specific games
  void hoi4Preparations(bool useDefaultStates, bool useDefaultProvinces);
  // generic preparations. However, if desired, there are necessary preparations
  // for every game such as reading in the existing worldmap, states, regions,
  // provinces etc
  void generateWorld();
  // map base continents to generic paradox compatible game continents
  void mapContinents();
  // map base regions to generic paradox compatible game regions
  void mapRegions();
  // calculating populations in states
  void generatePopulations();
  // determine development from habitability, population density and randomness
  void generateDevelopment();
  // mapping terrain types of FastWorldGen to paradox compatible terrains
  void mapTerrain();
  // GameRegions are used for every single game,
  Region &findStartRegion();
  // and countries are always created the same way
  void generateCountries(int numCountries);
  // see which country neighbours which
  void evaluateNeighbours();
};
} // namespace Scenario
