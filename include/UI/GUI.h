#pragma once
#include "ArdaUI/ArdaUI.h"
#include "FastWorldGenerator.h"
#include "eu4/Eu4Generator.h"
#include "hoi4/Hoi4Generator.h"
#include "misc/cpp/imgui_stdlib.h"
#include "utils/RpxUtils.h"
#include "vic3/Splnet.h"
#include "vic3/Vic3Generator.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <d3d11.h>
#include <string>
#include <tchar.h>
#include <vector>

// Short alias for this namespace
namespace pt = boost::property_tree;
struct GameConfig {
  std::string gameName;
  std::string gameShortName;
};

using Hoi4Gen = Rpx::Hoi4::Generator;
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
  int showScenarioTab(Fwg::Cfg &cfg);
  void countryEdit(std::shared_ptr<Arda::ArdaGen> generator);
  int showCountryTab(Fwg::Cfg &cfg);

  int showModuleGeneric(Fwg::Cfg &cfg);
  int showStrategicRegionTab(Fwg::Cfg &cfg,
                             std::shared_ptr<Rpx::ModGenerator> &generator);
  // to display the terrain maps/masks etc
  int showTerrainTab(Fwg::Cfg &cfg, std::shared_ptr<Arda::ArdaGen> generator);
  // HOI stuff
  int showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator);
  void pathWarning(std::exception e);
  int showHoi4Finalise(Fwg::Cfg &cfg);
  // Vic3 stuff
  int showVic3Configure(Fwg::Cfg &cfg, std::shared_ptr<Vic3Gen> generator);
  void showSplineTab(Fwg::Cfg &cfg);
  int showVic3Finalise(Fwg::Cfg &cfg);

  // generic stuff
  int showStatisticsTab();

  void recover();
  ;

  // std::vector<VisualLayer> layers = {
  //     {"World Map", 0.1f, false, &activeModule->generator->worldMap},
  //     {"Provinces", 0.1f, false, &activeModule->generator->provinceMap},
  //     {"Regions", 0.1f, false, &activeModule->generator->regionMap},
  //     {"WorldCivMap", 0.1f, false,
  //      [&]() {
  //        return Fwg::Gfx::displayWorldCivilisationMap(
  //            activeModule->generator->climateData,
  //            activeModule->generator->provinceMap,
  //            activeModule->generator->worldMap,
  //            activeModule->generator->civLayer,
  //            activeModule->generator->regionMap, "");
  //      }},
  //     {"Climate", 0.1f, false, &activeModule->generator->climateMap},
  // };

public:
  GUI();
  void genericWrapper();
  void gameSpecificTabs(Fwg::Cfg &cfg);
  int shiny(const pt::ptree &rpdConf, const std::string &configSubFolder,
            const std::string &username);
};
