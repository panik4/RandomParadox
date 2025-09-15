#pragma once
#include "ArdaGen.h"
#include "eu4/Eu4ImageExporter.h"
#include "eu4/Eu4Parsing.h"
#include "eu4/Eu4Region.h"
#include <generic/ModGenerator.h>

namespace Rpx::Eu4 {

struct Eu4Config {};
struct Eu4Data {};
struct Eu4Stats {};
class Generator : public Rpx::ModGenerator {
  std::vector<eu4Region> eu4regions;
  Rpx::Gfx::Eu4::ImageExporter formatConverter;

public:
  Eu4Config eu4Config;
  Eu4Data modData;
  Eu4Stats eu4Stats;

  Generator(const std::string &configSubFolder,
            const boost::property_tree::ptree &rpdConf);
  // clear and create all the mod paths at each run
  bool createPaths();

  void configureModGen(const std::string &configSubFolder,
                       const std::string &username,
                       const boost::property_tree::ptree &rpdConf) override;
  void generateRegions(std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions);

  Fwg::Gfx::Bitmap mapTerrain() override;
  // initialize countries
  void mapCountries() override;
  std::vector<eu4Region> getEu4Regions() { return eu4regions; };

  virtual void generate();
  virtual void initImageExporter();
  virtual void writeTextFiles();
  virtual void writeImages();
};
} // namespace Rpx::Eu4