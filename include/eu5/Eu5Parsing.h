#include "eu5/Eu5Generator.h"
#include "parsing/ParserUtils.h"
#include <areas/ArdaProvince.h>
#include <areas/ArdaRegion.h>

namespace Rpx::Eu5::Parsing {
namespace Writing {
void writeMetadata(const std::string &path);

// void adj(const std::string &path);
// void defaultMap(
//     const std::string &path,
//     const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces);
// void defines(const std::string &pathOut);
// void heightmap(const std::string &path, const Fwg::Gfx::Bitmap &heightMap,
//                const Fwg::Gfx::Bitmap &packedHeightmap);
//
// void stateFiles(const std::string &path,
//                 const std::vector<std::shared_ptr<Region>> &regions);
// void provinceTerrains(
//     const std::string &path,
//     const std::vector<std::shared_ptr<Arda::ArdaProvince>> &provinces);
// void strategicRegions(
//     const std::string &path,
//     const std::vector<std::shared_ptr<Arda::SuperRegion>> &strategicRegions,
//     const std::vector<std::shared_ptr<Region>> &regions);
// void cultureCommon(const std::string &path,
//                    const std::vector<std::shared_ptr<Arda::Culture>>
//                    &cultures);
// void religionCommon(
//     const std::string &path,
//     const std::vector<std::shared_ptr<Arda::Religion>> &religions);
// void countryCommon(
//     const std::string &path,
//     const std::map<std::string, std::shared_ptr<Country>> &countries,
//     const std::vector<std::shared_ptr<Region>> &regions);
// void popsHistory(const std::string &path,
//                  const std::vector<std::shared_ptr<Region>> &regions);
// void stateHistory(const std::string &path,
//                   const std::vector<std::shared_ptr<Region>> &regions);
// void countryHistory(
//     const std::string &path,
//     const std::map<std::string, std::shared_ptr<Country>> &countries);
//
// void staticModifiers(
//     const std::string &path,
//     const std::vector<std::shared_ptr<Arda::Culture>> &cultures,
//     const std::vector<std::shared_ptr<Arda::Religion>> &religions);
//
// void splineNetwork(const std::string &path);
//
// void compatFile(const std::string &path);
// std::string compatRegions(const std::string &inFolder,
//                           const std::string &outPath,
//                           const std::vector<std::shared_ptr<Region>>
//                           &regions);
//
// void compatStratRegions(const std::string &inFolder, const std::string
// &outPath,
//                         const std::vector<std::shared_ptr<Region>> &regions,
//                         std::string &baseArdaRegions);
// void compatReleasable(const std::string &inFolder, const std::string
// &outPath);
//
// void compatTriggers(const std::string &inFolder, const std::string &outPath);
//
// void locators(const std::string &path,
//               const std::vector<std::shared_ptr<Region>> &regions);

} // namespace Writing

namespace History {
// void writeBuildings(const std::string &path,
//                     const std::vector<std::shared_ptr<Region>> &regions);
}

}; // namespace Rpx::Eu5::Parsing