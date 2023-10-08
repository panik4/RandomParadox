#pragma once
#include "FastWorldGenerator.h"
#include "generic/FormatConverter.h"
#include "generic/GenericModule.h"
#include "hoi4/Hoi4Generator.h"
#include "hoi4/Hoi4MapPainting.h"
#include "hoi4/Hoi4Parsing.h"
#include "hoi4/NationalFocus.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>

namespace Scenario::Hoi4 {
class Hoi4Module : public GenericModule {

  // read the config specific to hoi4
  void readHoiConfig(const std::string &configSubFolder,
                     const std::string &username,
                     const boost::property_tree::ptree &rpdConf);

public:
  // Hoi4::Generator generator;
  std::shared_ptr<Hoi4::Generator> hoi4Gen;
  Hoi4Module(const boost::property_tree::ptree &rpdConf,
             const std::string &configSubFolder, const std::string &username,
             const bool editMode);
  ~Hoi4Module();
  // member functions
  void prepareData();
  void readHoi(std::string &gamePath);
  void mapCountries(bool multiCore, bool stateExport,
                    const std::string &inputMap);
  void mapEdit();
  void writeTextFiles();
  void writeImages();
  // clear and create all the mod paths at each run
  virtual bool createPaths();
  virtual void generate();
};
} // namespace Scenario::Hoi4