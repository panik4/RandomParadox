#pragma once
#include "FastWorldGenerator.h"
#include "Flag.h"
#include "GameProvince.h"
#include "GameRegion.h"
#include "NameGenerator.h"
#include "PdoxContinent.h"
#include "PdoxCountry.h"
#include "RandNum/RandNum.h"
#include "ResourceLoading.h"
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

  std::map<std::string, Fwg::Province::TerrainType> stringToTerrainType =
      {{"plains", Fwg::Province::TerrainType::grassland},
       {"forest", Fwg::Province::TerrainType::forest},
       {"ocean", Fwg::Province::TerrainType::ocean},
       {"mountain", Fwg::Province::TerrainType::mountains},
       {"marsh", Fwg::Province::TerrainType::marsh},
       {"hills", Fwg::Province::TerrainType::hills},
       {"jungle", Fwg::Province::TerrainType::jungle},
       {"desert", Fwg::Province::TerrainType::desert},
       {"urban", Fwg::Province::TerrainType::urban},
       {"lakes", Fwg::Province::TerrainType::lake}};
  std::map<Fwg::Province::TerrainType, std::string> terrainTypeToString =
      {{Fwg::Province::TerrainType::grassland, "plains"},
       {Fwg::Province::TerrainType::savannah, "plains"},
       {Fwg::Province::TerrainType::peaks, "mountain"},
       {Fwg::Province::TerrainType::marsh, "marsh"},
       {Fwg::Province::TerrainType::forest, "forest"},
       {Fwg::Province::TerrainType::ocean, "ocean"},
       {Fwg::Province::TerrainType::mountains, "mountain"},
       {Fwg::Province::TerrainType::hills, "hills"},
       {Fwg::Province::TerrainType::jungle, "jungle"},
       {Fwg::Province::TerrainType::desert, "desert"},
       {Fwg::Province::TerrainType::urban, "urban"},
       {Fwg::Province::TerrainType::lake, "lakes"}};

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
}; // namespace Scenario
} // namespace Scenario
