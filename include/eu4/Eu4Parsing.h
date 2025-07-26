#include "eu4/Eu4Generator.h"
#include "parsing/ParserUtils.h"
#include <areas/GameProvince.h>
#include <areas/GameRegion.h>

namespace Scenario::Eu4::Parsing {
std::string loadVanillaFile(const std::string &path,
                            const std::vector<std::string> &&filters);
void writeAdj(const std::string &path,
              const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeAmbientObjects(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
// areas consist of multiple provinces
void writeAreas(const std::string &path,
                const std::vector<std::shared_ptr<Region>> &regions,
                const std::string &gamePath);
void writeClimate(const std::string &path,
                  const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeColonialRegions(
    const std::string &path, const std::string &gamePath,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
// continents consist of multiple provinces
void writeContinent(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeDefaultMap(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeDefinition(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writePositions(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
// regions consist of multiple areas
void writeRegions(const std::string &path, const std::string &gamePath,
                  const std::vector<eu4Region> &eu4regions);
// superregions consist of multiple regions
void writeSuperregion(const std::string &path, const std::string &gamePath,
                      const std::vector<std::shared_ptr<Region>> &regions);
void writeTerrain(const std::string &path,
                  const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeTradeCompanies(
    const std::string &path, const std::string &gamePath,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
void writeTradewinds(
    const std::string &path,
    const std::vector<std::shared_ptr<GameProvince>> &provinces);
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName);

void writeProvinces(const std::string &path,
                    const std::vector<std::shared_ptr<GameProvince>> &provinces,
                    const std::vector<std::shared_ptr<Region>> &regions);

void writeLoc(const std::string &path, const std::string &gamePath,
              const std::vector<std::shared_ptr<Region>> &regions,
              const std::vector<std::shared_ptr<GameProvince>> &provinces,
              const std::vector<eu4Region> &eu4regions);
}; // namespace Scenario::Eu4::Parsing