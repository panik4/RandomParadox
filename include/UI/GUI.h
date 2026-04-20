#pragma once
#include "ArdaUI/ArdaUI.h"
#include "Hoi4UI.h"
#include "FastWorldGenerator.h"
#include "eu4/Eu4Generator.h"
#include "eu5/Eu5Generator.h"
#include "hoi4/Hoi4Generator.h"
#include "RpxPrerequisiteChecker.h"
#include "misc/cpp/imgui_stdlib.h"
#include "stb_image.h"
#include "utils/RpxUtils.h"
#include "vic3/Splnet.h"
#include "vic3/Vic3Generator.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <string>
#include <vector>

// Short alias for this namespace
namespace pt = boost::property_tree;
struct GameConfig {
  std::string gameName;
  std::string gameShortName;
};

using Vic3Gen = Rpx::Vic3::Generator;
using Eu4Gen = Rpx::Eu4::Generator;

class GUI : public Arda::ArdaUI {
  std::vector<GameConfig> gameConfigs;
  GameConfig activeGameConfig;
  pt::ptree rpdConf;
  std::string configSubFolder;
  std::string username;
  // std::shared_ptr<Rpx::GenericModule> activeModule;
  std::shared_ptr<Rpx::ModGenerator> activeGenerator;
  bool loadedConfigs = false;
  bool validatedPaths = false;
  std::string activeConfig;
  std::vector<std::string> configSubfolders;
  void loadConfigs();
  // configuration
  void loadGameConfig(Fwg::Cfg &cfg);
  void initGameConfigs();
  bool validatePaths();
  bool isRelevantModuleActive(const std::string &shortName);
  template <class T> constexpr std::shared_ptr<T> getGeneratorPointer() {
    return std::reinterpret_pointer_cast<T, Arda::ArdaGen>(activeGenerator);
  }
  int showConfigure(Fwg::Cfg &cfg);
  int showRpdxConfigure(Fwg::Cfg &cfg);
  void showModLoader(Fwg::Cfg &cfg);
  // generic scenario stuff
  void countryEdit(std::shared_ptr<Arda::ArdaGen> generator);
  void countryDrag(std::shared_ptr<Arda::ArdaGen> generator);
  int showCountryTab(Fwg::Cfg &cfg);

  int showModuleGeneric(Fwg::Cfg &cfg);
  void stratRegionEdit(std::shared_ptr<Arda::ArdaGen> generator);
  int showStrategicRegionTab(Fwg::Cfg &cfg,
                             std::shared_ptr<Rpx::ModGenerator> &generator);
  // HOI stuff
  void pathWarning(std::exception e);
  int showHoi4Finalise(Fwg::Cfg &cfg);
  // Vic3 stuff
  int showVic3Configure(Fwg::Cfg &cfg, std::shared_ptr<Vic3Gen> generator);
  void showSplineTab(Fwg::Cfg &cfg);
  int showVic3Finalise(Fwg::Cfg &cfg);
  void showEu5Finalise(Fwg::Cfg &cfg);

  // generic stuff
  int showStatisticsTab();

  void recover();

public:
  GUI();
  void genericWrapper();
  void gameSpecificTabs(Fwg::Cfg &cfg);
  int shiny(const pt::ptree &rpdConf, const std::string &configSubFolder,
            const std::string &username);
};
