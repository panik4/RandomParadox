#include "eu4/Eu4ScenarioGenerator.h"
#include "generic/ParserUtils.h"
#include <generic/GameProvince.h>
#include <generic/GameRegion.h>

class Eu4Parser {
  using pU = ParserUtils;
  static std::vector<std::string> locKeys;

public:
  static std::string loadVanillaFile(const std::string &path,
                                     const std::vector<std::string> &&filters);
  static void writeAdj(const std::string &path,
                       const std::vector<GameProvince> &provinces);
  static void writeAmbientObjects(const std::string &path,
                                  const std::vector<GameProvince> &provinces);
  // areas consist of multiple provinces
  static void writeAreas(const std::string &path,
                         const std::vector<GameRegion> &regions,
                         const std::string &gamePath);
  static void writeClimate(const std::string &path,
                           const std::vector<GameProvince> &provinces);
  static void writeColonialRegions(const std::string &path,
                                   const std::string &gamePath,
                                   const std::vector<GameProvince> &provinces);
  // continents consist of multiple provinces
  static void writeContinent(const std::string &path,
                             const std::vector<GameProvince> &provinces);
  static void writeDefaultMap(const std::string &path,
                              const std::vector<GameProvince> &provinces);
  static void writeDefinition(const std::string &path,
                              const std::vector<GameProvince> &provinces);
  static void writePositions(const std::string &path,
                             const std::vector<GameProvince> &provinces);
  // regions consist of multiple areas
  static void writeRegions(const std::string &path, const std::string &gamePath,
                           const std::vector<eu4Region> &eu4regions);
  // superregions consist of multiple regions
  static void writeSuperregion(const std::string &path,
                               const std::string &gamePath,
                               const std::vector<GameRegion> &regions);
  static void writeTerrain(const std::string &path,
                           const std::vector<GameProvince> &provinces);
  static void writeTradeCompanies(const std::string &path,
                                  const std::string &gamePath,
                                  const std::vector<GameProvince> &provinces);
  static void writeTradewinds(const std::string &path,
                              const std::vector<GameProvince> &provinces);
  static void copyDescriptorFile(const std::string &sourcePath,
                                 const std::string &destPath,
                                 const std::string &modsDirectory,
                                 const std::string &modName);

  static void writeProvinces(const std::string &path,
                             const std::vector<GameProvince> &provinces,
                             const std::vector<GameRegion> &regions);

  static void writeLoc(const std::string &path, const std::string &gamePath,
                       const std::vector<GameRegion> &regions,
                       const std::vector<GameProvince> &provinces,
                       const std::vector<eu4Region> &eu4regions);
};