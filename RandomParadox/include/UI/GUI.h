#pragma once
#include "FastWorldGenerator.h"
#include "UI/fwgUI.h"
#include "eu4/Eu4Module.h"
#include "generic/ScenarioUtils.h"
#include "hoi4/Hoi4Generator.h"
#include "hoi4/Hoi4Module.h"
#include "misc/cpp/imgui_stdlib.h"
#include "vic3/Vic3Module.h"
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

using Hoi4Gen = Scenario::Hoi4::Generator;
using Vic3Gen = Scenario::Vic3::Generator;
using Eu4Gen = Scenario::Eu4::Generator;

class GUI : Fwg::fwgUI {
  std::vector<GameConfig> gameConfigs;
  GameConfig activeGameConfig;
  pt::ptree rpdConf;
  std::string configSubFolder;
  std::string username;
  std::shared_ptr<Scenario::GenericModule> activeModule;
  bool loadedConfigs = false;
  bool configuredScenarioGen = false;
  bool validatedPaths = false;
  Fwg::Gfx::Bitmap regionSelectMap;
  Scenario::Utils::Pathcfg pathconfig;
  std::string activeConfig;
  std::vector<std::string> configSubfolders;
  std::vector<std::string> loadConfigs();
  // configuration
  void loadGameConfig(Fwg::Cfg &cfg);
  void initGameConfigs();
  bool isRelevantModuleActive(const std::string &shortName);
  int showGeneric(Fwg::Cfg &cfg, Scenario::Generator &generator,
                  ID3D11ShaderResourceView **texture);
  template <class T> constexpr std::shared_ptr<T> getGeneratorPointer() {
    return std::reinterpret_pointer_cast<T, Scenario::Generator>(
        activeModule->generator);
  }
  int showConfigure(Fwg::Cfg &cfg,
                    std::shared_ptr<Scenario::GenericModule> &genericModule);
  int showRpdxConfigure(
      Fwg::Cfg &cfg, std::shared_ptr<Scenario::GenericModule> &genericModule);
  void showModLoader(Fwg::Cfg &cfg,
                     std::shared_ptr<Scenario::GenericModule> &genericModule);
  bool scenarioGenReady();
  // generic scenario stuff
  int showScenarioTab(Fwg::Cfg &cfg,
                      std::shared_ptr<Scenario::GenericModule> genericModule);
  int showCountryTab(Fwg::Cfg &cfg, ID3D11ShaderResourceView **texture);

  // HOI stuff
  int showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator);
  int showModuleGeneric(Fwg::Cfg &cfg,
                        std::shared_ptr<Scenario::GenericModule> genericModule);
  int showStrategicRegionTab(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator);
  int showHoi4Finalise(Fwg::Cfg &cfg,
                       std::shared_ptr<Scenario::Hoi4::Hoi4Module> hoi4Module);
  int showStatisticsTab();

public:
  GUI();
  int shiny(const pt::ptree &rpdConf, const std::string &configSubFolder,
            const std::string &username);
};
