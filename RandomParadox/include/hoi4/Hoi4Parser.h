#pragma once
#include "FastWorldGenerator.h"
#include "generic/NameGenerator.h"
#include "generic/ParserUtils.h"
#include "generic/TextureWriter.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4ScenarioGenerator.h"
#include <array>

class Hoi4Parser {
  using pU = ParserUtils;
  using hoiMap = std::map<std::string, Hoi4Country>;
  static std::vector<std::string> defaultTags;

public:
  static std::string getBuildingLine(const std::string type,
                                     const Region &region, const bool coastal,
                                     const Bitmap &heightmap);
  // map
  static void dumpAdj(const std::string path);
  static void dumpAdjacencyRules(const std::string path);
  static void dumpAirports(const std::string path,
                           const std::vector<Region> &regions);
  static void dumpBuildings(const std::string path,
                            const std::vector<Region> &regions);
  static void dumpContinents(const std::string path,
                             const std::vector<Continent> &continents);
  static void dumpDefinition(const std::string path,
                             const std::vector<GameProvince> &provinces);
  static void dumpRocketSites(const std::string path,
                              const std::vector<Region> &regions);
  static void
  dumpStrategicRegions(const std::string path,
                       const std::vector<Region> &regions,
                       const std::vector<strategicRegion> strategicRegions);
  static void
  dumpSupply(const std::string path,
             const std::vector<std::vector<int>> supplyNodeConnections);
  static void dumpUnitStacks(const std::string path,
                             const std::vector<Province *> provinces);
  static void
  dumpWeatherPositions(const std::string path,
                       const std::vector<Region> &regions,
                       const std::vector<strategicRegion> strategicRegions);
  // gfx
  static void dumpFlags(const std::string path, const hoiMap &countries);

  // history
  static void dumpStates(const std::string path, const hoiMap &countries);
  static void
  writeHistoryCountries(const std::string path,
                                    const hoiMap &countries);
  static void
  writeHistoryUnits(const std::string path,
                                const hoiMap &countries);
  // history - National Focus
  static std::vector<std::string> readTypeMap();
  static std::map<std::string, std::string>
  readRewardMap(const std::string path);
  static void writeFoci(const std::string path, const hoiMap &countries);

  // common
  static void dumpCommonBookmarks(
      const std::string path,
      const hoiMap &countries,
      const std::map<int, std::vector<std::string>> strengthScores);
  static void
  dumpCommonCountries(const std::string path, const std::string hoiPath,
                      const hoiMap &countries);
  static void
  dumpCommonCountryTags(const std::string path,
                        const hoiMap &countries);

  // localisation
  static void
  writeCountryNames(const std::string path,
                    const hoiMap &countries);
  static void
  writeStateNames(const std::string path,
                  const hoiMap &countries);
  static void writeStrategicRegionNames(
      const std::string path,
      const std::vector<strategicRegion> strategicRegions);

  // copy base game countries and remove certain lines to reduce crashes
  static void writeCompatibilityHistory(const std::string path,
                                        const std::string hoiPath,
                                        const std::vector<Region> &regions);

  // copy over mod descriptor file
  static void copyDescriptorFile(const std::string sourcePath,
                                 const std::string destPath,
                                 const std::string modsDirectory,
                                 const std::string modName);
};
