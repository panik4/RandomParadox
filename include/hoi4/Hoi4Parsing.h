#pragma once
#include "FastWorldGenerator.h"
#include "namegeneration/NameGenerator.h"
#include "parsing/ParserUtils.h"
#include "io/Textures.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4Generator.h"
#include <array>
#include <map>
namespace Scenario::Hoi4::Parsing {
using CountryMap = std::vector<std::shared_ptr<Hoi4Country>>;

namespace Writing {
namespace Map {

void adj(const std::string &path);
void adjacencyRules(const std::string &path);
void ambientObjects(const std::string &path);
void buildings(const std::string &path,
               const std::vector<std::shared_ptr<Region>> &regions);

void continents(const std::string &path,
                const std::vector<Arda::ArdaContinent> &continents,
                const std::string &hoiPath,
                const std::string &localisationPath);
void definition(const std::string &path,
                const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces);
void strategicRegions(const std::string &path,
                      const std::vector<Fwg::Areas::Region> &regions,
                      const std::vector<StrategicRegion> &strategicRegions);
void supply(const std::string &path,
            const std::vector<std::vector<int>> &supplyNodeConnections);
void unitStacks(const std::string &path,
                const std::vector<std::shared_ptr<Arda::ArdaProvince>> provinces,
                const std::vector<std::shared_ptr<Region>> regions,
                const std::vector<float> &heightMap);
void weatherPositions(const std::string &path,
                      const std::vector<Fwg::Areas::Region> &regions,
                      std::vector<StrategicRegion> &strategicRegions);

} // namespace Map

namespace Countries {
// common
void commonCountries(const std::string &path, const std::string &hoiPath,
                     const CountryMap &countries);
void commonCountryTags(const std::string &path, const CountryMap &countries);

void commonCharacters(const std::string &path, const CountryMap &countries);
void commonNames(const std::string &path, const CountryMap &countries);
void foci(const std::string &path, const CountryMap &countries,
          const NameGeneration::NameData &nData);
// gfx
void flags(const std::string &path, const CountryMap &countries);
// history
void states(const std::string &path,
            const std::vector<std::shared_ptr<Region>> &regions);
void historyCountries(const std::string &path, const CountryMap &countries,
                      const std::string &gamePath,
                      const std::vector<Fwg::Areas::Region> &regions);
void historyUnits(const std::string &path, const CountryMap &countries);
void ideas(const std::string &path, const CountryMap &countries);

// portraits
void portraits(const std::string &path, const CountryMap &countries);

} // namespace Countries

void aiStrategy(const std::string &path,
                const std::vector<Arda::ArdaContinent> &continents);
void events(const std::string &path);

// common
void commonBookmarks(
    const std::string &path, const CountryMap &countries,
    const std::map<int, std::vector<std::shared_ptr<Arda::Country>>>
        &strengthScores);

void tutorials(const std::string &path);
// copy base game countries and remove certain lines to reduce crashes
void compatibilityHistory(const std::string &path, const std::string &hoiPath,
                          const std::vector<Fwg::Areas::Region> &regions);
void scriptedTriggers(std::string gamePath, std::string modPath);

void commonFiltering(const std::string &gamePath, const std::string &modPath);

// copy over mod descriptor file
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName);
// localisation
namespace Localisation {

void countryNames(const std::string &path, const CountryMap &countries,
                  const NameGeneration::NameData &nData);
void stateNames(const std::string &path, const CountryMap &countries);
void strategicRegionNames(const std::string &path,
                          const std::vector<StrategicRegion> &strategicRegions);
void victoryPointNames(const std::string &path,
                       const std::vector<std::shared_ptr<Region>> &regions);

} // namespace Localisation

} // namespace Writing

namespace Reading {

struct ChangeHolder;

Fwg::Utils::ColourTMap<std::string> readColourMapping(const std::string &path);
void readStates(const std::string &path, std::shared_ptr<Generator> &hoi4Gen);

std::vector<Fwg::Areas::Province> readProvinceMap(const std::string &path);
void readAirports(const std::string &path,
                  std::vector<std::shared_ptr<Region>> &regions);

void readAdj(const std::string &path, Fwg::Areas::AreaData &areaData);
void readBuildings(const std::string &path,
                   std::vector<std::shared_ptr<Region>> &regions);
std::vector<std::shared_ptr<Hoi4Country>>
readCountries(const std::string &path);
std::vector<std::vector<std::string>> readDefinitions(const std::string &path);
void readProvinces(const Fwg::Terrain::TerrainData &terrainData,
                   Fwg::Climate::ClimateData &climateData,
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
} // namespace Scenario::Hoi4::Parsing