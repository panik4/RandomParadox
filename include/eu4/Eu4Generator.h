#pragma once
#include "eu4/Eu4FormatConverter.h"
#include "eu4/Eu4Parsing.h"
#include "eu4/Eu4Region.h"
#include <generic/ModGenerator.h>
#include "ArdaGen.h"

namespace Rpx::Eu4 {


class Generator : public Rpx::ModGenerator {
  std::vector<eu4Region> eu4regions;
  Rpx::Gfx::Eu4::FormatConverter formatConverter;

public:
  Generator(const std::string &configSubFolder,
            const boost::property_tree::ptree &rpdConf);
  // clear and create all the mod paths at each run
  bool createPaths();

  void configureModGen(const std::string &configSubFolder,
                       const std::string &username,
                       const boost::property_tree::ptree &rpdConf) override;
  void generateRegions(std::vector<std::shared_ptr<Arda::ArdaRegion>> &regions);

  // initialize countries
  virtual void mapCountries();
  std::vector<eu4Region> getEu4Regions() { return eu4regions; };

  virtual void generate();
  virtual void initFormatConverter();
  virtual void writeTextFiles();
  virtual void writeImages();
};
} // namespace Rpx::Eu4