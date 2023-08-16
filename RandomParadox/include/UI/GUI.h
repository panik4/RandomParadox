#pragma once
#include "FastWorldGenerator.h"
#include "UI/fwgUI.h"
#include "generic/ScenarioUtils.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

class GUI : Fwg::fwgUI {
  bool loadedConfigs = false;
  std::string activeConfig;
  std::vector<std::string> configSubfolders;
  std::vector<std::string> loadConfigs();
  int showConfigure(Fwg::Cfg &cfg, ID3D11ShaderResourceView **texture);
  int showCountryTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                     ID3D11ShaderResourceView **texture);
  int showStrategicRegionTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                             ID3D11ShaderResourceView **texture);
  void loadGameConfig(Fwg::Cfg &cfg);

public:
  int shiny(Fwg::FastWorldGenerator &fwg, Scenario::Utils::Pathcfg &pathconfig);
};
