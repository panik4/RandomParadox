#pragma once
#include "FastWorldGenerator.h"
#include "UI/fwgUI.h"
#include "eu4/Eu4Module.h"
#include "hoi4/Hoi4Generator.h"
#include "hoi4/Hoi4Module.h"
#include "misc/cpp/imgui_stdlib.h"
#include "utils/RpxUtils.h"
#include "vic3/Splnet.h"
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

using Hoi4Gen = Rpx::Hoi4::Generator;
using Vic3Gen = Rpx::Vic3::Generator;
using Eu4Gen = Rpx::Eu4::Generator;

class GUI : Fwg::FwgUI {
  enum class VisualLayerType {
    HEIGHTMAP,
    ELEVATIONTYPES,
    TOPOGRAPHY,
    NORMALMAP,
    INCLINATION,
    RELSURROUNDINGS,
    HUMIDITY,
    TEMPERATURE,
    CLIMATE,
    TREECLIMATE,
    DENSITY,
    SUPERSEGMENTS,
    SEGMENTS,
    PROVINCES,
    REGIONS,
    REGIONSWITHPROVINCES,
    REGIONSWITHBORDERS,
    CONTINENTS,
    POPULATION,
    DEVELOPMENT,
    ARABLELAND,
    LOCATIONS,
    WORLD_MAP,
    CIVILISATION_MAP,
    SUPERREGIONS,
    COUNTRIES,
    CULTUREGROUPS,
    CULTURES,
    RELIGIONS
  };
  std::vector<GameConfig> gameConfigs;
  GameConfig activeGameConfig;
  pt::ptree rpdConf;
  std::string configSubFolder;
  std::string username;
  std::shared_ptr<Rpx::GenericModule> activeModule;
  bool loadedConfigs = false;
  bool configuredScenarioGen = false;
  bool validatedPaths = false;
  Fwg::Gfx::Bitmap regionSelectMap;
  std::string activeConfig;
  std::vector<std::string> configSubfolders;
  std::vector<std::string> loadConfigs();
  // configuration
  void loadGameConfig(Fwg::Cfg &cfg);
  void initGameConfigs();
  bool validatePaths();
  bool isRelevantModuleActive(const std::string &shortName);
  int showGeneric(Fwg::Cfg &cfg, Arda::ArdaGen &generator,
                  ID3D11ShaderResourceView **texture);
  template <class T> constexpr std::shared_ptr<T> getGeneratorPointer() {
    return std::reinterpret_pointer_cast<T, Arda::ArdaGen>(
        activeModule->generator);
  }
  int showConfigure(Fwg::Cfg &cfg,
                    std::shared_ptr<Rpx::GenericModule> &genericModule);
  int showRpdxConfigure(Fwg::Cfg &cfg,
                        std::shared_ptr<Rpx::GenericModule> &genericModule);
  void showModLoader(Fwg::Cfg &cfg,
                     std::shared_ptr<Rpx::GenericModule> &genericModule);
  bool scenarioGenReady(bool printIssue);
  std::string getLayerTypeName(VisualLayerType type);
  bool showVisualLayerToggles(std::map<VisualLayerType, bool> &layerVisibility);
  // generic scenario stuff
  int showScenarioTab(Fwg::Cfg &cfg,
                      std::shared_ptr<Rpx::GenericModule> genericModule);
  void countryEdit(std::shared_ptr<Arda::ArdaGen> generator);
  int showCountryTab(Fwg::Cfg &cfg);

  int showModuleGeneric(Fwg::Cfg &cfg,
                        std::shared_ptr<Rpx::GenericModule> genericModule);
  int showStrategicRegionTab(Fwg::Cfg &cfg,
                             std::shared_ptr<Rpx::ModGenerator> generator);
  // to display the terrain maps/masks etc
  int showTerrainTab(Fwg::Cfg &cfg, std::shared_ptr<Arda::ArdaGen> generator);
  // HOI stuff
  int showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator);
  void pathWarning(std::exception e);
  int showHoi4Finalise(Fwg::Cfg &cfg,
                       std::shared_ptr<Rpx::Hoi4::Hoi4Module> hoi4Module);
  // Vic3 stuff
  int showVic3Configure(Fwg::Cfg &cfg, std::shared_ptr<Vic3Gen> generator);
  void showSplineTab(Fwg::Cfg &cfg,
                     std::shared_ptr<Rpx::Vic3::Module> vic3Module);
  int showVic3Finalise(Fwg::Cfg &cfg,
                       std::shared_ptr<Rpx::Vic3::Module> vic3Module);

  // generic stuff
  int showStatisticsTab();

  void recover();

  std::map<VisualLayerType, bool> visualLayerVisibility = {
      {VisualLayerType::HEIGHTMAP, false},
      {VisualLayerType::ELEVATIONTYPES, false},
      {VisualLayerType::TOPOGRAPHY, false},
      {VisualLayerType::HUMIDITY, false},
      {VisualLayerType::TEMPERATURE, false},
      {VisualLayerType::CLIMATE, false},
      {VisualLayerType::TREECLIMATE, false},
      {VisualLayerType::DENSITY, false},
      {VisualLayerType::SUPERSEGMENTS, false},
      {VisualLayerType::SEGMENTS, false},
      {VisualLayerType::PROVINCES, false},
      {VisualLayerType::REGIONS, false},
      {VisualLayerType::CONTINENTS, false},
      {VisualLayerType::POPULATION, false},
      {VisualLayerType::DEVELOPMENT, false},
      {VisualLayerType::LOCATIONS, false},
      {VisualLayerType::WORLD_MAP, false},
      {VisualLayerType::CIVILISATION_MAP, false}};

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
