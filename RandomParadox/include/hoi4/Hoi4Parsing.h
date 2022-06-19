#pragma once
#include "FastWorldGenerator.h"
#include "generic/NameGenerator.h"
#include "generic/ParserUtils.h"
#include "generic/Textures.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4Generator.h"
#include <array>

namespace Hoi4::Parsing {
using pU = ParserUtils;
using hoiMap = std::map<std::string, Hoi4::Hoi4Country>;
static std::vector<std::string> defaultTags;

std::string getBuildingLine(const std::string &type, const Region &region,
                            const bool coastal, const Bitmap &heightmap);
// map
void dumpAdj(const std::string &path);
void dumpAdjacencyRules(const std::string &path);
void dumpAirports(const std::string &path, const std::vector<Region> &regions);
void dumpBuildings(const std::string &path, const std::vector<Region> &regions);
void dumpContinents(const std::string &path,
                    const std::vector<Continent> &continents);
void dumpDefinition(const std::string &path,
                    const std::vector<GameProvince> &provinces);
void dumpRocketSites(const std::string &path,
                     const std::vector<Region> &regions);
void dumpStrategicRegions(const std::string &path,
                          const std::vector<Region> &regions,
                          const std::vector<strategicRegion> &strategicRegions);
void dumpSupply(const std::string &path,
                const std::vector<std::vector<int>> supplyNodeConnections);
void dumpUnitStacks(const std::string &path,
                    const std::vector<Province *> provinces);
void dumpWeatherPositions(const std::string &path,
                          const std::vector<Region> &regions,
                          const std::vector<strategicRegion> &strategicRegions);
// gfx
void dumpFlags(const std::string &path, const hoiMap &countries);

// history
void dumpStates(const std::string &path, const hoiMap &countries);
void writeHistoryCountries(const std::string &path, const hoiMap &countries);
void writeHistoryUnits(const std::string &path, const hoiMap &countries);
// history - National Focus
std::vector<std::string> readTypeMap();
std::map<std::string, std::string> readRewardMap(const std::string &path);
void writeFoci(const std::string &path, const hoiMap &countries);

// common
void dumpCommonBookmarks(
    const std::string &path, const hoiMap &countries,
    const std::map<int, std::vector<std::string>> &strengthScores);
void dumpCommonCountries(const std::string &path, const std::string &hoiPath,
                         const hoiMap &countries);
void dumpCommonCountryTags(const std::string &path, const hoiMap &countries);

// localisation
void writeCountryNames(const std::string &path, const hoiMap &countries);
void writeStateNames(const std::string &path, const hoiMap &countries);
void writeStrategicRegionNames(
    const std::string &path,
    const std::vector<strategicRegion> &strategicRegions);

// copy base game countries and remove certain lines to reduce crashes
void writeCompatibilityHistory(const std::string &path,
                               const std::string &hoiPath,
                               const std::vector<Region> &regions);

// copy over mod descriptor file
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName);
} // namespace Hoi4::Parsing