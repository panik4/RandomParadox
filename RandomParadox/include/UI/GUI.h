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

class GUI : Fwg::fwgUI {
  bool loadedConfigs = false;
  bool configuredScenarioGen = false;
  Scenario::Utils::Pathcfg pathconfig;
  std::string activeConfig;
  std::vector<std::string> configSubfolders;
  std::vector<std::string> loadConfigs();
  int showConfigure(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module);
  int showRpdxConfigure(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module);
  int showHoi4Configure(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module,
                        ID3D11ShaderResourceView **texture);

  int showCountryTab(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module,
                     ID3D11ShaderResourceView **texture);
  int showStrategicRegionTab(Fwg::Cfg &cfg,
                             Scenario::Hoi4::Generator &generator,
                             ID3D11ShaderResourceView **texture);
  int showFinaliseTabConfigure(Fwg::Cfg &cfg,
                               Scenario::Hoi4::Hoi4Module &hoi4Module);
  int showStatisticsTab();
  void loadGameConfig(Fwg::Cfg &cfg);

public:
  int shiny(Scenario::Hoi4::Hoi4Module &hoi4Module);
};
