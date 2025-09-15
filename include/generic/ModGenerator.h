#pragma once
#include "ArdaGen.h"
#include "generic/StrategicRegion.h"
#include "io/GameDataImporter.h"
#include "namegeneration/NameGenerator.h"
#include "rendering/Images.h"
#include <map>
namespace Rpx {

class ModGenerator : public Arda::ArdaGen {

public:
  // vars - used for every game
  GameType gameType = GameType::Generic;

  Utils::Pathcfg pathcfg;
  // constructors&destructor
  ModGenerator(const std::string &configSubFolder, const GameType &gameType,
               const std::string &gameSubpath,
               const boost::property_tree::ptree &rpdConf);
  ~ModGenerator();
  virtual bool createPaths() = 0;

  virtual void configureModGen(const std::string &configSubFolder,
                               const std::string &username,
                               const boost::property_tree::ptree &rpdConf) = 0;

  // mapping terrain types of FastWorldGen to module
  // compatible terrains
  virtual Fwg::Gfx::Bitmap mapTerrain() = 0;
  // initialize countries
  virtual void mapCountries();

  virtual void generate() = 0;
  virtual void initImageExporter() = 0;
  virtual void writeTextFiles() = 0;
  virtual void writeImages() = 0;

}; // namespace Rpx
} // namespace Rpx
