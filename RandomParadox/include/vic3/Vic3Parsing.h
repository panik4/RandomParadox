#include "generic/ParserUtils.h"
#include "vic3/Vic3Generator.h"
#include <generic/GameProvince.h>
#include <generic/GameRegion.h>

namespace Scenario::Vic3::Parsing {
namespace Writing {
void adj(const std::string &path);
void defaultMap(const std::string &path,
                const std::vector<std::shared_ptr<GameProvince>> &provinces);
void heightmap(const std::string &path, const Fwg::Gfx::Bitmap &heightMap);

void stateFiles(const std::string &path,
                const std::vector<std::shared_ptr<Region>> &regions);
void provinceTerrains(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeMetadata(const std::string &path);

void strategicRegions(const std::string &path,
                      const std::vector<strategicRegion> &strategicRegions,
                      const std::vector<std::shared_ptr<Region>> &regions);
void cultureCommon(const std::string &path,
                   const std::vector<std::shared_ptr<Culture>> &cultures);
void religionCommon(const std::string &path,
                    const std::vector<std::shared_ptr<Religion>> &religions);
void countryCommon(const std::string &path,
                   const std::map<std::string, PdoxCountry> &countries,
                   const std::vector<std::shared_ptr<Region>> &regions);
void popsHistory(const std::string &path,
                  const std::vector<std::shared_ptr<Region>> &regions);
void stateHistory(const std::string &path,
                  const std::vector<std::shared_ptr<Region>> &regions);
void countryHistory(const std::string &path,
                   const std::map<std::string, PdoxCountry> &countries);

} // namespace Writing

namespace Reading {}

}; // namespace Scenario::Vic3::Parsing