#pragma once
#include "FastWorldGenerator.h"
#include "generic/NameGenerator.h"
#include "generic/ParserUtils.h"
#include "generic/Textures.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4Generator.h"
#include <array>
namespace Scenario::Hoi4::Parsing {
using CountryMap = std::vector<std::shared_ptr<Hoi4Country>>;

namespace Writing {
void adj(const std::string &path);
void adjacencyRules(const std::string &path);
void airports(const std::string &path, const std::vector<Fwg::Region> &regions);
void aiStrategy(const std::string &path, const CountryMap &countries);
void events(const std::string &path, const CountryMap &countries);
void ambientObjects(const std::string &path, const Fwg::Gfx::Bitmap &heightMap);
void buildings(const std::string &path,
               const std::vector<std::shared_ptr<Region>> &regions,
               const Fwg::Gfx::Bitmap &heightMap);
void continents(const std::string &path,
                const std::vector<Fwg::Continent> &continents);
void definition(const std::string &path,
                const std::vector<std::shared_ptr<GameProvince>> &provinces);
void rocketSites(const std::string &path,
                 const std::vector<Fwg::Region> &regions);
void strategicRegions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      const std::vector<StrategicRegion> &strategicRegions);
void supply(const std::string &path,
            const std::vector<std::vector<int>> &supplyNodeConnections);
void unitStacks(const std::string &path,
                const std::vector<Fwg::Province *> provinces,
                const std::vector<std::shared_ptr<Region>> regions,
                const Fwg::Gfx::Bitmap &heightMap);
void weatherPositions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      std::vector<StrategicRegion> &strategicRegions);
// gfx
void flags(const std::string &path, const CountryMap &countries);

// history
void states(const std::string &path,
            const std::vector<std::shared_ptr<Region>> &regions);
void historyCountries(const std::string &path, const CountryMap &countries);
void historyUnits(const std::string &path, const CountryMap &countries);
void foci(const std::string &path, const CountryMap &countries,
          const NameGeneration::NameData &nData);

// common
void commonBookmarks(
    const std::string &path, const CountryMap &countries,
    const std::map<int, std::vector<std::shared_ptr<Country>>> &strengthScores);
void commonCountries(const std::string &path, const std::string &hoiPath,
                     const CountryMap &countries);
void commonCountryTags(const std::string &path, const CountryMap &countries);
void commonNames(const std::string &path, const CountryMap &countries);
// localisation
void countryNames(const std::string &path, const CountryMap &countries,
                  const NameGeneration::NameData &nData);
void stateNames(const std::string &path, const CountryMap &countries);
void strategicRegionNames(const std::string &path,
                          const std::vector<StrategicRegion> &strategicRegions);
void victoryPointNames(const std::string &path,
                       const std::vector<std::shared_ptr<Region>> &regions);
void tutorials(const std::string &path);
// copy base game countries and remove certain lines to reduce crashes
void compatibilityHistory(const std::string &path, const std::string &hoiPath,
                          const std::vector<Fwg::Region> &regions);
void scriptedTriggers(std::string gamePath, std::string modPath);

void commonFiltering(const std::string &gamePath, const std::string &modPath);

// portraits
void portraits(const std::string &path, const CountryMap &countries);

} // namespace Writing

namespace Reading {

struct ChangeHolder;

Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path);
void readStates(const std::string &path, std::shared_ptr<Generator> &hoi4Gen);

std::vector<Fwg::Province> readProvinceMap(const std::string &path);
void readAirports(const std::string &path,
                  std::vector<std::shared_ptr<Region>> &regions);

void readAdj(const std::string &path, Fwg::Areas::AreaData &areaData);
void readBuildings(const std::string &path,
                   std::vector<std::shared_ptr<Region>> &regions);
std::vector<std::shared_ptr<Hoi4Country>>
readCountries(const std::string &path);
std::vector<std::vector<std::string>> readDefinitions(const std::string &path);
void readProvinces(Fwg::ClimateGeneration::ClimateData &climateData,
                   const std::string &inPath, const std::string &mapName,
                   Fwg::Areas::AreaData &areaData);
void readRailways(const std::string &path, Fwg::Areas::AreaData &areaData);
void readRocketSites(const std::string &path,
                     std::vector<std::shared_ptr<Region>> &regions);
void readSupplyNodes(const std::string &path,
                     std::vector<std::shared_ptr<Region>> &regions);
void readUnitStacks(const std::string &path, Fwg::Areas::AreaData &areaData);
void readWeatherPositions(const std::string &path,
                          std::vector<std::shared_ptr<Region>> &regions);
} // namespace Reading

// history - National Focus
std::vector<std::string> readTypeMap();
std::map<std::string, std::string> readRewardMap(const std::string &path);
// copy over mod descriptor file
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName);
} // namespace Scenario::Hoi4::Parsing