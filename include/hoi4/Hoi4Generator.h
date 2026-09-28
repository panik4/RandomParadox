#pragma once
#include "FastWorldGenerator.h"
#include "areas/SuperRegion.h"
#include "countries/Country.h"
#include "generic/ModGenerator.h"
#include "generic/StrategicRegion.h"
#include "hoi4/Hoi4Army.h"
#include "hoi4/Hoi4Country.h"
#include "hoi4/Hoi4CountryGen.h"
#include "hoi4/Hoi4DataStructures.h"
#include "hoi4/Hoi4DecisionGen.h"
#include "hoi4/Hoi4ForceGen.h"
#include "hoi4/Hoi4ImageExporter.h"
#include "hoi4/Hoi4Parsing.h"
#include "hoi4/Hoi4ProvGen.h"
#include "hoi4/Hoi4Region.h"
#include "hoi4/Hoi4StateGen.h"
#include "hoi4/Hoi4SupplyGen.h"
#include "hoi4/Hoi4TechGen.h"
#include "io/GenericParsing.h"
#include "utils/RpxUtils.h"
#include <array>
#include <set>

namespace Rpx::Hoi4 {

class Generator : public Rpx::ModGenerator {
  // a hoi4 specific image exporter
  Gfx::Hoi4::ImageExporter imageExporter;
  Hoi4Stats stats;

public:
  Hoi4Config modConfig;
  Hoi4Data modData;

  // constructors/destructors
  Generator(const std::string &configSubFolder,
            const boost::property_tree::ptree &rpdConf);
  ~Generator();
  // member functions
  bool createPaths();
  void configureModGen(const std::string &configSubFolder,
                       const std::string &username,
                       const boost::property_tree::ptree &rpdConf) override;

  void mapRegions();
  virtual Fwg::Gfx::Image mapTerrain();
  // initialize states
  void mapCountries();

  void generateStateSpecifics();
  void generateStateResources();

  // politics: names, ideology, party popularity, elections
  void generateSoftCountryDetails();
  // area dependent country specifics: unit gen, tech, characters, army/navy/air
  // focus, naval bases, airports, logistics
  void generateHardCountrySpecifics();
  void deriveCountrySpecificsFromSimulation();
  void balanceGreatPowers();
  void generateAndBalanceFactions();
  // generate weather per strategic region, from baseprovinces
  void generateWeather();

  // calculate how strong each country is
  void finaliseData();

  // print world info to console
  void printStatistics();

  void loadStates();
  virtual bool loadRivers(Fwg::Cfg &config,
                          const Fwg::Gfx::Image &riverInput) override;
  virtual bool loadRiversFromBlueMask(
      Fwg::Cfg &config, const Fwg::Gfx::Image &riverInput) override;

  virtual void generate();
  virtual void initImageExporter();
  void writeLocalisation();
  virtual void writeTextFiles(bool scenarioDetails);
  virtual void writeImages();
  void save(const std::string &path) override;
  void load(const std::string &path) override;
  const Gfx::Hoi4::ImageExporter &getImageExporter() const {
    return imageExporter;
  }

  void readHoi(std::string &gamePath);
};
} // namespace Rpx::Hoi4