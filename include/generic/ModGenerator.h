#pragma once
#include "ArdaGen.h"
#include "io/ResourceLoading.h"
#include "namegeneration/NameGenerator.h"
#include "rendering/Images.h"
#include <map>
namespace Rpx {
class StrategicRegion : public Arda::SuperRegion {
public:
  // weather: month{averageTemp, standard deviation, average precipitation,
  // tempLow, tempHigh, tempNightly, snowChance, lightRainChance,
  // heavyRainChance, blizzardChance,mudChance, sandstormChance}
  std::vector<std::vector<double>> weatherMonths;
};

class ModGenerator : public Arda::ArdaGen {

public:
  // vars - used for every game
  ModGenerator();
  ModGenerator(const std::string &configSubFolder);
  ModGenerator(Arda::ArdaGen &scenGen);
  ~ModGenerator();

  // mapping terrain types of FastWorldGen to module
  // compatible terrains
  virtual Fwg::Gfx::Bitmap mapTerrain();
  // initialize countries
  virtual void mapCountries();

  virtual void cutFromFiles(const std::string &gamePath);

}; // namespace Rpx
} // namespace Rpx
