#pragma once
#include "FastWorldGenerator.h"
#include "UI/fwgUI.h"
#include "generic/ScenarioUtils.h"
#include "hoi4/Hoi4Generator.h"
#include "hoi4/Hoi4Module.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "misc/cpp/imgui_stdlib.h"
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

class GUI : Fwg::fwgUI {
  std::vector<GameConfig> gameConfigs;
  GameConfig activeGameConfig;
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
  int showConfigure(Fwg::Cfg &cfg,
                    std::shared_ptr<Scenario::GenericModule> genericModule);
  int showRpdxConfigure(Fwg::Cfg &cfg,
                        std::shared_ptr<Scenario::GenericModule> genericModule);
  // generic scenario stuff
  int showScenarioTab(Fwg::Cfg &cfg,
                      std::shared_ptr<Scenario::GenericModule> genericModule);
  int showCountryTab(Fwg::Cfg &cfg, ID3D11ShaderResourceView **texture);

  // HOI stuff
  int showHoi4Configure(Fwg::Cfg &cfg,
                        std::shared_ptr<Scenario::Hoi4::Generator> generator);
  int showHoiGeneric(Fwg::Cfg &cfg,
                     std::shared_ptr<Scenario::Hoi4::Hoi4Module> hoi4Module);
  int showStateTab(Fwg::Cfg &cfg,
                   std::shared_ptr<Scenario::Hoi4::Generator> generator);
  int showStrategicRegionTab(
      Fwg::Cfg &cfg, std::shared_ptr<Scenario::Hoi4::Generator> generator);
  int showHoi4Finalise(Fwg::Cfg &cfg,
                       std::shared_ptr<Scenario::Hoi4::Hoi4Module> hoi4Module);
  int showStatisticsTab();

public:
  int shiny(const pt::ptree &rpdConf, const std::string &configSubFolder,
            const std::string &username);
};
