#include "generic/ParserUtils.h"
#include "vic3/Vic3Generator.h"
#include <generic/GameProvince.h>
#include <generic/GameRegion.h>

namespace Scenario::Vic3::Parsing {
namespace Writing {
void adj(const std::string &path);
void defaultMap(const std::string &path,
                const std::vector<std::shared_ptr<GameProvince>> &provinces);
void defines(const std::string &pathOut);
void heightmap(const std::string &path, const Fwg::Gfx::Bitmap &heightMap,
               const Fwg::Gfx::Bitmap &packedHeightmap);

void stateFiles(const std::string &path,
                const std::vector<std::shared_ptr<Region>> &regions);
void provinceTerrains(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeMetadata(const std::string &path);

void strategicRegions(const std::string &path,
                      const std::vector<StrategicRegion> &strategicRegions,
                      const std::vector<std::shared_ptr<Region>> &regions);
void cultureCommon(const std::string &path,
                   const std::vector<std::shared_ptr<Culture>> &cultures);
void religionCommon(const std::string &path,
                    const std::vector<std::shared_ptr<Religion>> &religions);
void countryCommon(
    const std::string &path,
    const std::map<std::string, std::shared_ptr<Country>> &countries,
    const std::vector<std::shared_ptr<Region>> &regions);
void popsHistory(const std::string &path,
                 const std::vector<std::shared_ptr<Region>> &regions);
void stateHistory(const std::string &path,
                  const std::vector<std::shared_ptr<Region>> &regions);
void countryHistory(
    const std::string &path,
    const std::map<std::string, std::shared_ptr<Country>> &countries);

void staticModifiers(const std::string &path,
                     const std::vector<std::shared_ptr<Culture>> &cultures,
                     const std::vector<std::shared_ptr<Religion>> &religions);

void splineNetwork(const std::string &path);

void compatFile(const std::string &path);
std::string compatRegions(const std::string &inFolder,
                          const std::string &outPath,
                          const std::vector<std::shared_ptr<Region>> &regions);

void compatStratRegions(const std::string &inFolder, const std::string &outPath,
                        const std::vector<std::shared_ptr<Region>> &regions,
                        std::string &baseGameRegions);
void compatReleasable(const std::string &inFolder, const std::string &outPath);

void compatTriggers(const std::string &inFolder, const std::string &outPath);

void locators(const std::string &path,
              const std::vector<std::shared_ptr<Region>> &regions);

} // namespace Writing

namespace History {
void writeBuildings(const std::string &path,
                    const std::vector<std::shared_ptr<Region>> &regions);
}

}; // namespace Scenario::Vic3::Parsing