#include "generic/ParserUtils.h"
#include <generic/GameProvince.h>
#include <generic/GameRegion.h>

class Eu4Parser {
  using pU = ParserUtils;

public:
  static void writeAdj(const std::string path,
                       const std::vector<GameProvince> &provinces);
  static void writeAmbientObjects(const std::string path,
                                  const std::vector<GameProvince> &provinces);
  // areas consist of multiple provinces
  static void writeAreas(const std::string path,
                         const std::vector<GameRegion> &regions);
  static void writeClimate(const std::string path,
                           const std::vector<GameProvince> &provinces);
  // continents consist of multiple provinces
  static void writeContinent(const std::string path,
                             const std::vector<GameProvince> &provinces);
  static void writeDefaultMap(const std::string path,
                              const std::vector<GameProvince> &provinces);
  static void writeDefinition(const std::string path,
                              const std::vector<GameProvince> &provinces);
  static void writePositions(const std::string path,
                             const std::vector<GameProvince> &provinces);
  // regions consist of multiple areas
  static void writeRegions(const std::string path,
                               const std::vector<GameRegion> &regions);
  // superregions consist of multiple regions
  static void writeSuperregion(const std::string path,
                               const std::vector<GameRegion> &regions);
  static void writeTerrain(const std::string path,
                           const std::vector<GameProvince> &provinces);
  static void writeTradewinds(const std::string path,
                           const std::vector<GameProvince> &provinces);
};