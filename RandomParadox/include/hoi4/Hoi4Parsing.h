#pragma once
#include "FastWorldGenerator.h"
#include "generic/NameGenerator.h"
#include "generic/ParserUtils.h"
#include "generic/Textures.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4Generator.h"
#include <array>
namespace Scenario::Hoi4::Parsing {
using pU = ParserUtils;
using hoiMap = std::map<std::string, Hoi4::Hoi4Country>;

namespace Writing {
void adj(const std::string &path);
void adjacencyRules(const std::string &path);
void airports(const std::string &path, const std::vector<Fwg::Region> &regions);
void buildings(const std::string &path,
               const std::vector<Fwg::Region> &regions);
void continents(const std::string &path,
                const std::vector<Fwg::Continent> &continents);
void definition(const std::string &path,
                const std::vector<GameProvince> &provinces);
void rocketSites(const std::string &path,
                 const std::vector<Fwg::Region> &regions);
void strategicRegions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      const std::vector<strategicRegion> &strategicRegions);
void supply(const std::string &path,
            const std::vector<std::vector<int>> supplyNodeConnections);
void unitStacks(const std::string &path,
                const std::vector<Fwg::Province *> provinces);
void weatherPositions(const std::string &path,
                      const std::vector<Fwg::Region> &regions,
                      const std::vector<strategicRegion> &strategicRegions);
// gfx
void flags(const std::string &path, const hoiMap &countries);

// history
void states(const std::string &path, const hoiMap &countries);
void historyCountries(const std::string &path, const hoiMap &countries);
void historyUnits(const std::string &path, const hoiMap &countries);
void foci(const std::string &path, const hoiMap &countries);

// common
void commonBookmarks(
    const std::string &path, const hoiMap &countries,
    const std::map<int, std::vector<std::string>> &strengthScores);
void commonCountries(const std::string &path, const std::string &hoiPath,
                     const hoiMap &countries);
void commonCountryTags(const std::string &path, const hoiMap &countries);

// localisation
void countryNames(const std::string &path, const hoiMap &countries);
void stateNames(const std::string &path, const hoiMap &countries);
void strategicRegionNames(const std::string &path,
                          const std::vector<strategicRegion> &strategicRegions);

// copy base game countries and remove certain lines to reduce crashes
void compatibilityHistory(const std::string &path, const std::string &hoiPath,
                          const std::vector<Fwg::Region> &regions);

} // namespace Writing

std::string getBuildingLine(const std::string &type, const Fwg::Region &region,
                            const bool coastal, const Fwg::Gfx::Bitmap &heightmap);
// history - National Focus
std::vector<std::string> readTypeMap();
std::map<std::string, std::string> readRewardMap(const std::string &path);
// copy over mod descriptor file
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName);
} // namespace Scenario::Hoi4::Parsing