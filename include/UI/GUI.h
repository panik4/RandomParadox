#pragma once
#include "ArdaUI/ArdaUI.h"
#include "Hoi4UI.h"
#include "MapEditorState.h"
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
#include <stdexcept>
#include <string>
#include <vector>

// Short alias for this namespace
namespace pt = boost::property_tree;
struct GameConfig {
  GameType type;
  std::string gameName;
  std::string gameShortName;
};

using Vic3Gen = Rpx::Vic3::Generator;

class GUI : public Arda::ArdaUI {
  std::vector<GameConfig> gameConfigs;
  pt::ptree rpdConf;
  std::string configSubFolder;
  std::string username;
  std::shared_ptr<Rpx::ModGenerator> activeGenerator;
  bool pathsInitialized = false;
  bool validatedPaths = false;
  Rpx::UI::MapEditorState mapEditorState;
  bool strategicRegionBorderPaint = false;
  int selectedStrategicRegionID = -1;
  // configuration
  void initGameConfigs();
  std::shared_ptr<Rpx::ModGenerator> createGenerator(GameType gameType) const;
  const GameConfig &gameConfig(GameType gameType) const;
  const GameConfig &activeGameConfig() const;
  void selectGame(GameType gameType);
  bool validatePaths();
  template <class T> std::shared_ptr<T> getGeneratorPointer() {
    auto generator = std::dynamic_pointer_cast<T>(activeGenerator);
    if (!generator)
      throw std::logic_error("Active generator type does not match the "
                             "requested game-specific operation");
    return generator;
  }
  int showConfigure(Fwg::Cfg &cfg);
  int showRpdxConfigure(Fwg::Cfg &cfg);
  // generic scenario stuff
  void processMapEditorSelection();
  std::shared_ptr<Arda::ArdaRegion> getSelectedMapEditorState() const;
  std::shared_ptr<Arda::ArdaProvince> getSelectedMapEditorProvince() const;
  std::shared_ptr<Fwg::Civilization::Location>
  getSelectedMapEditorLocation() const;
  std::shared_ptr<Arda::ArdaContinent> getSelectedMapEditorContinent() const;
  std::shared_ptr<Arda::Country> getSelectedMapEditorCountry(
      const std::shared_ptr<Arda::ArdaRegion> &state) const;
  const Rpx::UI::MapEditorModule *getMapEditorModule();
  void showGenericCountryEditor(const std::shared_ptr<Arda::Country> &country);
  void showGenericStateEditor(const std::shared_ptr<Arda::ArdaRegion> &state);
  void showGenericAreaNameEditor(const std::shared_ptr<Fwg::Areas::Area> &area,
                                 const char *label);
  void showGenericContinentSummary(
      const std::shared_ptr<Arda::ArdaContinent> &continent,
      Fwg::UI::Elements::GridLayout &grid);
  void applyCountryBorderPaint(
      const std::shared_ptr<Arda::ArdaRegion> &state);
  void refreshMapEditorVisualization();
  void refreshStrategicRegionVisualization();
  void showMapEditorLayer();
  void countryEdit();
  void stateEdit();
  void countryDrag();
  int showCountryTab(Fwg::Cfg &cfg);

  int showModuleGeneric(Fwg::Cfg &cfg);
  void stratRegionEdit();
  int showStrategicRegionTab(Fwg::Cfg &cfg,
                             std::shared_ptr<Rpx::ModGenerator> &generator);
  // HOI stuff
  void pathWarning(std::exception e, const Rpx::ModGenerator &generator);
  int showHoi4Finalise(Fwg::Cfg &cfg);
  // Vic3 stuff
  int showVic3Configure(Fwg::Cfg &cfg, std::shared_ptr<Vic3Gen> generator);
  void showSplineTab(Fwg::Cfg &cfg);
  int showVic3Finalise(Fwg::Cfg &cfg);
  void showEu5Finalise(Fwg::Cfg &cfg);

protected:
  Rpx::ModGenerator &currentGenerator() override;
  std::string windowTitle() const override;
  std::string frameTitle() const override;
  void onWindowReady() override;
  void renderTopBar() override;
  void renderSettingsTabs(Fwg::Cfg &cfg) override;
  void renderGenericContents(Fwg::Cfg &cfg) override;

public:
  GUI();
  void gameSpecificTabs(Fwg::Cfg &cfg);
  int shiny(const pt::ptree &rpdConf, const std::string &configSubFolder,
            const std::string &username);
};
