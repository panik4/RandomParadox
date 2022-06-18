#include "eu4/Eu4ScenarioGenerator.h"
#include "generic/ParserUtils.h"
#include <generic/GameProvince.h>
#include <generic/GameRegion.h>

namespace Eu4::Parser {
using pU = ParserUtils;
std::string loadVanillaFile(const std::string &path,
                            const std::vector<std::string> &&filters);
void writeAdj(const std::string &path,
              const std::vector<GameProvince> &provinces);
void writeAmbientObjects(const std::string &path,
                         const std::vector<GameProvince> &provinces);
// areas consist of multiple provinces
void writeAreas(const std::string &path, const std::vector<GameRegion> &regions,
                const std::string &gamePath);
void writeClimate(const std::string &path,
                  const std::vector<GameProvince> &provinces);
void writeColonialRegions(const std::string &path, const std::string &gamePath,
                          const std::vector<GameProvince> &provinces);
// continents consist of multiple provinces
void writeContinent(const std::string &path,
                    const std::vector<GameProvince> &provinces);
void writeDefaultMap(const std::string &path,
                     const std::vector<GameProvince> &provinces);
void writeDefinition(const std::string &path,
                     const std::vector<GameProvince> &provinces);
void writePositions(const std::string &path,
                    const std::vector<GameProvince> &provinces);
// regions consist of multiple areas
void writeRegions(const std::string &path, const std::string &gamePath,
                  const std::vector<eu4Region> &eu4regions);
// superregions consist of multiple regions
void writeSuperregion(const std::string &path, const std::string &gamePath,
                      const std::vector<GameRegion> &regions);
void writeTerrain(const std::string &path,
                  const std::vector<GameProvince> &provinces);
void writeTradeCompanies(const std::string &path, const std::string &gamePath,
                         const std::vector<GameProvince> &provinces);
void writeTradewinds(const std::string &path,
                     const std::vector<GameProvince> &provinces);
void copyDescriptorFile(const std::string &sourcePath,
                        const std::string &destPath,
                        const std::string &modsDirectory,
                        const std::string &modName);

void writeProvinces(const std::string &path,
                    const std::vector<GameProvince> &provinces,
                    const std::vector<GameRegion> &regions);

void writeLoc(const std::string &path, const std::string &gamePath,
              const std::vector<GameRegion> &regions,
              const std::vector<GameProvince> &provinces,
              const std::vector<eu4Region> &eu4regions);
}; // namespace Eu4::Parser