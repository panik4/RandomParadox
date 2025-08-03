#include "UI/GUI.h"
// Data
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
// to track which game was selected for generation
static int selectedGame = 0;
static bool showErrorPopup = false;
static std::string errorLog;
static bool requireCountryDetails = false;
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from
// your application based on those two flags.
// LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
  case WM_SIZE:
    if (wParam == SIZE_MINIMIZED)
      return 0;
    g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
    g_ResizeHeight = (UINT)HIWORD(lParam);
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
// for state/country/strategic region editing
static bool drawBorders = false;
void GUI::recover() {
  auto &cfg = Fwg::Cfg::Values();
  auto &generator = *activeModule->generator;
  generator.terrainData.deserialize(cfg.mapsPath + "terrainData.bin");
  generator.climateData.deserialize(cfg.mapsPath + "climateData.bin");
  generator.areaData.deserialize(cfg.mapsPath + "areaData.bin");
}
GUI::GUI() : Arda::ArdaUI() {}

void GUI::genericWrapper() {
  auto &cfg = Fwg::Cfg::Values();
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
  {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 100, 144, 40));

    ImGui::BeginChild(
        "GenericWrapper",
        ImVec2(
            ImGui::GetContentRegionAvail().x * 1.0f,
            std::max<float>(ImGui::GetContentRegionAvail().y * 0.3f, 100.0f)),
        false, window_flags);
    if (!validatedPaths)
      ImGui::BeginDisabled();
    showGeneric(cfg, *activeModule->generator);
    if (!validatedPaths)
      ImGui::EndDisabled();
    ImGui::SameLine();
    showModuleGeneric(cfg, activeModule);
    ImGui::EndChild();
    // Draw a frame around the child region
    ImVec2 childMin = ImGui::GetItemRectMin();
    ImVec2 childMax = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddRect(
        childMin, childMax, IM_COL32(50, 91, 120, 255), 0.0f, 0, 2.0f);
    ImGui::PopStyleColor();
  }
}

void GUI::gameSpecificTabs(Fwg::Cfg &cfg) {
  if (!scenarioGenReady(false)) {
    ImGui::BeginDisabled();
  }
  if (activeGameConfig.gameName == "Hearts of Iron IV") {
    auto hoi4Gen = std::reinterpret_pointer_cast<Hoi4Gen, Rpx::ModGenerator>(
        activeModule->generator);
    showStrategicRegionTab(cfg, activeModule->generator);
    showCountryTab(cfg);
    showHoi4Finalise(
        cfg, std::reinterpret_pointer_cast<Rpx::Hoi4::Hoi4Module,
                                           Rpx::GenericModule>(activeModule));
  } else if (activeGameConfig.gameName == "Victoria 3") {
    auto vic3Gen = std::reinterpret_pointer_cast<Vic3Gen, Rpx::ModGenerator>(
        activeModule->generator);
    showStrategicRegionTab(cfg, activeModule->generator);
    showCountryTab(cfg);
    showNavmeshTab(cfg, *activeModule->generator);
    showVic3Finalise(
        cfg,
        std::reinterpret_pointer_cast<Rpx::Vic3::Module, Rpx::GenericModule>(
            activeModule));
  }
  if (!scenarioGenReady(false)) {
    ImGui::EndDisabled();
  }
}

int GUI::shiny(const pt::ptree &rpdConf, const std::string &configSubFolder,
               const std::string &username) {

  try {
    //  Create application window
    //  ImGui_ImplWin32_EnableDpiAwareness();
    auto wc = initializeWindowClass();

    HWND consoleWindow = GetConsoleWindow();

    ::RegisterClassExW(&wc);
    HWND hwnd = uiUtils->createAndConfigureWindow(wc, wc.lpszClassName,
                                                  L"RandomParadox 0.9.1");
    initializeGraphics(hwnd);
    initializeImGui(hwnd);
    auto &io = ImGui::GetIO();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    auto &cfg = Fwg::Cfg::Values();
    // Main loop
    bool done = false;
    //--- prior to main loop:
    DragAcceptFiles(hwnd, TRUE);
    uiUtils->primaryTexture = nullptr;
    uiUtils->device = g_pd3dDevice;

    // rpx related
    this->rpdConf = rpdConf;
    this->configSubFolder = configSubFolder;
    this->username = username;
    activeModule = std::make_shared<Rpx::Hoi4::Hoi4Module>(
        Rpx::Hoi4::Hoi4Module(rpdConf, configSubFolder, username, false));
    activeModule->generator->climateData.addSecondaryColours(
        Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                               "hoi4/colourMappings.txt"));
    initGameConfigs();

    activeModule->generator->configure(cfg);

    init(cfg, *activeModule->generator);

    while (!done) {
      try {
        initDraggingPoll(done);
        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
          ImGui::SetNextWindowPos({0, 0});
          ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
          ImGui::Begin("RandomParadox");
          if (!validatedPaths) {
            ImGui::TextColored({255, 0, 100, 255},
                               "You need to validate paths successfully before "
                               "being able to do anything else");
          }
          // observer checks for "Error"
          // auto errors = observer.pollForMessage("Error");
          // if (!errors.empty()) {
          //   errorLog = errors;
          //   showErrorPopup = true;
          //   ImGui::OpenPopup("Error Log");
          // }

          if (ImGui::BeginPopupModal("Error Log", NULL,
                                     ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped("%s", errorLog.c_str());

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Close", ImVec2(120, 0))) {
              ImGui::CloseCurrentPopup();
              showErrorPopup = false;
            }

            ImGui::EndPopup();
          }

          ImGui::BeginChild("LeftContent",
                            ImVec2(ImGui::GetContentRegionAvail().x * 0.4f,
                                   ImGui::GetContentRegionAvail().y * 1.0f),
                            false);
          {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(78, 90, 204, 40));
            // Create a child window for the left content
            ImGui::BeginChild("SettingsContent",
                              ImVec2(ImGui::GetContentRegionAvail().x * 1.0f,
                                     ImGui::GetContentRegionAvail().y * 0.8f),
                              false);
            {
              ImGui::SeparatorText(
                  "Different Steps of the generation, usually go "
                  "from left to right");

              if (ImGui::BeginTabBar("Steps", ImGuiTabBarFlags_None)) {
                // Disable all inputs if computation is running
                if (computationRunning) {
                  ImGui::BeginDisabled();
                }
                showConfigure(cfg, activeModule);
                if (!validatedPaths)
                  ImGui::BeginDisabled();
                if (cfg.debugLevel == 9) {
                  showModLoader(cfg, activeModule);
                }
                defaultTabs(cfg, *activeModule->generator);
                showScenarioTab(cfg, activeModule);
                gameSpecificTabs(cfg);
                auto ardaGen = std::static_pointer_cast<Arda::ArdaGen>(
                    activeModule->generator);
                overview(ardaGen, cfg);
                if (!validatedPaths)
                  ImGui::EndDisabled();
                // Re-enable inputs if computation is running
                if (computationRunning && !computationStarted) {
                  ImGui::EndDisabled();
                }
                // Check if the computation is done
                if (computationRunning &&
                    computationFutureBool.wait_for(std::chrono::seconds(0)) ==
                        std::future_status::ready) {
                  computationRunning = false;
                  uiUtils->resetTexture();
                }

                if (computationRunning) {
                  computationStarted = false;
                  ImGui::Text("Working, please be patient");
                } else {
                  ImGui::Text("Ready!");
                }

                ImGui::EndTabBar();
              }

              ImGui::PopStyleColor();
              ImGui::EndChild();
              // Draw a frame around the child region
              ImVec2 childMin = ImGui::GetItemRectMin();
              ImVec2 childMax = ImGui::GetItemRectMax();
              ImGui::GetWindowDrawList()->AddRect(childMin, childMax,
                                                  IM_COL32(100, 90, 180, 255),
                                                  0.0f, 0, 2.0f);
            }

            genericWrapper();
            logWrapper();
          }
          ImGui::SameLine();
          imageWrapper(io);
          ImGui::End();

          if (uiUtils->showExtendedHelp) {
            uiUtils->showAdvancedTextBox();
          }
        }

        // Rendering
        uiUtils->renderImGui(g_pd3dDeviceContext, g_mainRenderTargetView,
                             clear_color, g_pSwapChain);
      } catch (std::exception e) {
        Fwg::Utils::Logging::logLine("Error in GUI main loop: ", e.what());
      }
    }

    cleanup(hwnd, wc);
    return 0;
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine("Error in GUI startup: ", e.what());
    return -1;
  }
}

std::vector<std::string> GUI::loadConfigs() {
  std::vector<std::string> configSubfolders;
  for (const auto &entry : std::filesystem::directory_iterator(
           Fwg::Cfg::Values().workingDirectory + "//configs")) {
    if (entry.is_directory() && !entry.path().string().contains("heightmap")) {
      configSubfolders.push_back(entry.path().string());
      Fwg::Utils::Logging::logLine(entry);
    }
  }

  return configSubfolders;
}

void GUI::loadGameConfig(Fwg::Cfg &cfg) {
  namespace pt = boost::property_tree;
  pt::ptree hoi4Conf;
  try {
    // Read the basic settings
    std::ifstream f(activeConfig + "//Hearts of Iron IVModule.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Fwg::Parsing::replaceInStringStream(buffer, "//", "//");

    pt::read_json(buffer, hoi4Conf);
  } catch (std::exception e) {
    Fwg::Utils::Logging::logLine("Incorrect config \"RandomParadox.json\"");
    Fwg::Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                                 e.what());
    Fwg::Utils::Logging::logLine(
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
  }
}
// hardcoded init of some game configs
void GUI::initGameConfigs() {
  gameConfigs.push_back({"Hearts of Iron IV", "hoi4"});
  gameConfigs.push_back({"Victoria 3", "vic3"});
  gameConfigs.push_back({"Europa Universalis IV", "eu4"});
  activeGameConfig = gameConfigs[0];
}

bool GUI::validatePaths() {
  validatedPaths = activeModule->findGame(activeModule->pathcfg.gamePath,
                                          activeGameConfig.gameName);
  if (validatedPaths)
    validatedPaths =
        activeModule->validateGameModFolder(activeGameConfig.gameName);
  if (validatedPaths)
    validatedPaths = activeModule->validateModFolder(activeGameConfig.gameName);
  activeModule->initFormatConverter();
  return validatedPaths;
}

bool GUI::isRelevantModuleActive(const std::string &shortName) {
  return activeGameConfig.gameShortName == shortName;
}

// generic configure tab, containing a tab for fwg and rpdx configs
int GUI::showConfigure(Fwg::Cfg &cfg,
                       std::shared_ptr<Rpx::GenericModule> &activeModule) {

  if (ImGui::BeginTabItem("Configure")) {
    uiUtils->showHelpTextBox("Configure");
    if (ImGui::BeginTabBar("Config tabs", ImGuiTabBarFlags_None)) {
      showRpdxConfigure(cfg, activeModule);
      showFwgConfigure(cfg);
      ImGui::EndTabBar();
    }
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showRpdxConfigure(Fwg::Cfg &cfg,
                           std::shared_ptr<Rpx::GenericModule> &activeModule) {
  static int item_current = 0;
  // remove the images, and set pretext for them to be auto
  // loaded after switching tabs again
  if (ImGui::BeginTabItem("RandomParadox Configuration")) {
    ImGui::PushItemWidth(200.0f);
    uiUtils->tabSwitchEvent();
    // find every subfolder of config folder
    if (!loadedConfigs) {
      loadedConfigs = true;
      configSubfolders = loadConfigs();
      activeConfig = configSubfolders[item_current];
      // on startup, try to auto locate game and game mod folder, then auto
      // validate
      activeModule->findGame(activeModule->pathcfg.gamePath,
                             activeGameConfig.gameName);
      activeModule->autoLocateGameModFolder(activeGameConfig.gameName);
      validatePaths();
    }

    std::vector<const char *> gameSelection;
    for (auto &gameConfig : gameConfigs) {
      gameSelection.push_back(gameConfig.gameName.c_str());
    }
    if (ImGui::ListBox("Game Selection", &selectedGame, gameSelection.data(),
                       gameSelection.size())) {
      if (gameConfigs[selectedGame].gameName == "Hearts of Iron IV") {
        activeModule = std::make_shared<Rpx::Hoi4::Hoi4Module>(
            Rpx::Hoi4::Hoi4Module(rpdConf, configSubFolder, username, false));
        activeModule->generator->climateData.addSecondaryColours(
            Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                                   "hoi4/colourMappings.txt"));
      } else if (gameConfigs[selectedGame].gameName ==
                 "Europa Universalis IV") {
        activeModule = std::make_shared<Rpx::Eu4::Module>(
            Rpx::Eu4::Module(rpdConf, configSubFolder, username));
      } else if (gameConfigs[selectedGame].gameName == "Victoria 3") {
        activeModule = std::make_shared<Rpx::Vic3::Module>(
            Rpx::Vic3::Module(rpdConf, configSubFolder, username));
      }
      activeGameConfig = gameConfigs[selectedGame];
      activeModule->findGame(activeModule->pathcfg.gamePath,
                             activeGameConfig.gameName);
      activeModule->autoLocateGameModFolder(activeGameConfig.gameName);
      validatedPaths = false;
      validatePaths();
      activeModule->generator->configure(cfg);
    }
    std::vector<const char *> items;
    for (auto &item : configSubfolders)
      items.push_back(item.c_str());
    ImGui::SeparatorText(
        "Click an entry in the list to choose a config preset");
    if (ImGui::ListBox("Config Presets", &item_current, items.data(),
                       items.size())) {
      activeConfig = items[item_current];
      Fwg::Utils::Logging::logLine("Switched to ", activeConfig,
                                   "//FastWorldGenerator.json");
      cfg.readConfig(activeConfig);
      loadGameConfig(cfg);
      activeModule->generator->configure(cfg);
    }

    ImGui::PopItemWidth();
    ImGui::PushItemWidth(600.0f);
    ImGui::InputText("Mod Name", &activeModule->pathcfg.modName);
    ImGui::InputText("Game Path", &activeModule->pathcfg.gamePath);
    ImGui::InputText("Mod Path", &activeModule->pathcfg.gameModPath);
    ImGui::InputText("Mods Directory",
                     &activeModule->pathcfg.gameModsDirectory);
    if (ImGui::Button("Try to find game files")) {
      activeModule->findGame(activeModule->pathcfg.gamePath,
                             activeGameConfig.gameName);
    }
    ImGui::SameLine();
    if (ImGui::Button("Try to find the games mods folder")) {
      activeModule->autoLocateGameModFolder(activeGameConfig.gameName);
    }
    ImGui::SameLine();
    if (ImGui::Button("Validate all paths")) {
      validatePaths();
    }
    ImGui::PopItemWidth();
    ImGui::EndTabItem();
  }
  // force path for cutting to gamePath + maps, but only if no other path
  // defined
  if (cfg.cut) {
    cfg.loadMapsPath = activeModule->pathcfg.gamePath + "map//";
  }
  cfg.climateMappingPath = Fwg::Cfg::Values().resourcePath + "" +
                           activeGameConfig.gameShortName +
                           "//climateMapping.txt ";
  return 0;
}

void GUI::showModLoader(Fwg::Cfg &cfg,
                        std::shared_ptr<Rpx::GenericModule> &genericModule) {
  if (ImGui::BeginTabItem("Modloader")) {
    if (triggeredDrag) {
      auto hoi4Module =
          std::reinterpret_pointer_cast<Rpx::Hoi4::Hoi4Module,
                                        Rpx::GenericModule>(genericModule);
      hoi4Module->readHoi(draggedFile);
      triggeredDrag = false;
      uiUtils->resetTexture();
    }
    ImGui::EndTabItem();
  }
}

bool GUI::scenarioGenReady(bool printIssue) {
  auto ready = configuredScenarioGen && !redoRegions && !redoProvinces;
  const auto &generator = activeModule->generator;
  if (!generator->areaData.provinces.size() ||
      !generator->areaData.regions.size())
    return false;
  if (generator->areaData.provinces.size() != generator->ardaProvinces.size() ||
      generator->areaData.regions.size() != generator->ardaRegions.size() ||
      generator->areaData.provinces[0] !=
          generator->ardaProvinces[0]->baseProvince) {
    ready = false;
  }
  auto &cfg = Fwg::Cfg::Values();
  if (generator->civLayer.urbanisation.size() != cfg.bitmapSize ||
      generator->civLayer.agriculture.size() != cfg.bitmapSize) {
    if (printIssue) {
      Fwg::Utils::Logging::logLine("You seem to not have generated data in the "
                                   "civilisation tab, or it is "
                                   "of the wrong size");
    }
    ready = false;
  }
  if (!generator->civLayer.agriculture.size() ||
      !generator->locationMap.size()) {
    if (printIssue) {
      Fwg::Utils::Logging::logLine("You seem to not have generated data in the "
                                   "locations tab");
    }
    ready = false;
  }
  return ready;
}

int GUI::showScenarioTab(Fwg::Cfg &cfg,
                         std::shared_ptr<Rpx::GenericModule> activeModule) {
  int retCode = 0;
  if (ImGui::BeginTabItem("Scenario")) {
    if (uiUtils->tabSwitchEvent()) {
      uiUtils->updateImage(0, Fwg::Gfx::displayWorldCivilisationMap(
                                  activeModule->generator->climateData,
                                  activeModule->generator->provinceMap,
                                  activeModule->generator->worldMap,
                                  activeModule->generator->civLayer,
                                  activeModule->generator->regionMap, ""));
      uiUtils->updateImage(1, Fwg::Gfx::Bitmap());
    }

    // allow printing why the scenario generation is not ready
    scenarioGenReady(true);
    if (activeModule->generator->terrainData.detailedHeightMap.size() &&
        activeModule->generator->climateMap.initialised() &&
        activeModule->generator->provinceMap.initialised() &&
        activeModule->generator->regionMap.initialised() &&
        activeModule->generator->worldMap.initialised()) {
      // auto initialize
      ImGui::SeparatorText(
          "Only remap when you have changed the maps in the previous tabs");
      uiUtils->showHelpTextBox("Scenario");
      if (ImGui::Button("Remap areas")) {
        if (!activeModule->createPaths()) {
          Fwg::Utils::Logging::logLine("ERROR: Couldn't create paths");
          retCode = -1;
        }
        activeModule->initNameData(Fwg::Cfg::Values().resourcePath + "names",
                                   activeModule->pathcfg.gamePath);
        // start with the generic stuff in the Scenario Generator
        activeModule->generator->wrapup(cfg);
        activeModule->generator->mapProvinces();
        activeModule->generator->mapRegions();
        activeModule->generator->mapTerrain();
        activeModule->generator->mapContinents();
        Arda::Civilization::generateWorldCivilizations(
            activeModule->generator->ardaRegions,
            activeModule->generator->ardaProvinces,
            activeModule->generator->civData,
            activeModule->generator->scenContinents, activeModule->generator->superRegions);

        configuredScenarioGen = true;
      }
      ImGui::PushItemWidth(200.0f);
      ImGui::InputDouble("WorldPopulationFactor",
                         &activeModule->generator->worldPopulationFactor, 0.1);
      ImGui::InputDouble("industryFactor",
                         &activeModule->generator->worldIndustryFactor, 0.1);
      if (isRelevantModuleActive("hoi4")) {
        auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
        showHoi4Configure(cfg, hoi4Gen);
      } else if (isRelevantModuleActive("vic3")) {
        auto vic3Gen = getGeneratorPointer<Vic3Gen>();
        showVic3Configure(cfg, vic3Gen);
      } else if (isRelevantModuleActive("eu4")) {
        auto eu4Gen = getGeneratorPointer<Eu4Gen>();
      }
      ImGui::PopItemWidth();
    } else {
      ImGui::Text("Generate required maps in the other tabs first");
    }
    ImGui::EndTabItem();
  }
  return retCode;
}

void GUI::countryEdit(std::shared_ptr<Arda::ArdaGen> generator) {
  static int selectedStateIndex = 0;
  static std::string drawCountryTag;
  if (!drawBorders) {
    drawCountryTag = "";
  }
  auto &clickEvents = uiUtils->clickEvents;
  if (clickEvents.size()) {
    auto pix = clickEvents.front();
    clickEvents.pop();
    const auto &colour = generator->provinceMap[pix.pixel];
    if (generator->areaData.provinceColourMap.find(colour)) {
      const auto &prov = generator->areaData.provinceColourMap[colour];
      if (prov->regionID < generator->ardaRegions.size()) {
        auto &state = generator->ardaRegions[prov->regionID];
        selectedStateIndex = state->ID;
      }
    }
  }
  if (generator->ardaRegions.size()) {
    auto &modifiableState = generator->ardaRegions[selectedStateIndex];

    if ((modifiableState->owner &&
             generator->countries.find(modifiableState->owner->tag) !=
                 generator->countries.end() ||
         (drawCountryTag.size()) && generator->countries.find(drawCountryTag) !=
                                        generator->countries.end())) {
      std::shared_ptr selectedCountry =
          modifiableState->owner ? modifiableState->owner
                                 : generator->countries.at(drawCountryTag);
      if (!drawBorders) {
        drawCountryTag = selectedCountry->tag;
      }
      std::string tempTag = selectedCountry->tag;
      static std::string bufferChangedTag = "";

      Elements::borderChild("CountryEdit", [&]() {
        if (ImGui::InputText("Country tag", &tempTag)) {
          bufferChangedTag = tempTag;
        }
        if (ImGui::Button("update tag")) {
          if (bufferChangedTag.size() != 3) {
            Fwg::Utils::Logging::logLine("Tag must be 3 characters long");
          } else {
            std::string &oldTag = selectedCountry->tag;
            if (oldTag == bufferChangedTag) {
              Fwg::Utils::Logging::logLine("Tag is the same as the old one");
            } else {
              generator->countries.erase(oldTag);
              selectedCountry->tag = bufferChangedTag;
              // add country under different tag
              generator->countries.insert(
                  {selectedCountry->tag, selectedCountry});
              for (auto &region : selectedCountry->ownedRegions) {
                region->owner = selectedCountry;
              }
            }
            requireCountryDetails = true;
            generator->visualiseCountries(generator->countryMap);
          }
        }
        ImGui::InputText("Country name", &selectedCountry->name);
        ImGui::InputText("Country adjective", &selectedCountry->adjective);
        ImVec4 color =
            ImVec4(((float)selectedCountry->colour.getRed()) / 255.0f,
                   ((float)selectedCountry->colour.getGreen()) / 255.0f,
                   ((float)selectedCountry->colour.getBlue()) / 255.0f, 1.0f);

        if (ImGui::ColorEdit3("Country Colour", (float *)&color,
                              ImGuiColorEditFlags_NoInputs |
                                  ImGuiColorEditFlags_NoLabel |
                                  ImGuiColorEditFlags_HDR)) {
          selectedCountry->colour = Fwg::Gfx::Colour(
              color.x * 255.0, color.y * 255.0, color.z * 255.0);
          generator->visualiseCountries(generator->countryMap);
          uiUtils->resetTexture();
        }
      });

      if (drawBorders && drawCountryTag.size()) {
        if (generator->countries.find(drawCountryTag) !=
            generator->countries.end()) {
          selectedCountry = generator->countries.at(drawCountryTag);
        }
        if (selectedCountry != nullptr && !modifiableState->isSea() &&
            modifiableState->owner != selectedCountry) {
          modifiableState->owner->removeRegion(modifiableState);
          modifiableState->owner = selectedCountry;
          selectedCountry->addRegion(modifiableState);
          requireCountryDetails = true;
          generator->visualiseCountries(generator->countryMap,
                                        modifiableState->ID);
          uiUtils->updateImage(0, generator->countryMap);
        }
      }
    }

    Elements::borderChild("StateEdit", [&]() {
      if (ImGui::InputText("State name", &modifiableState->name)) {
        requireCountryDetails = true;
      }
      if (modifiableState->owner) {
        ImGui::Text("State owner", &modifiableState->owner->tag);
      }
      if (ImGui::InputInt("Population", &modifiableState->totalPopulation)) {
        requireCountryDetails = true;
      }
    });

    if (isRelevantModuleActive("hoi4")) {
      const auto &hoi4Region =
          std::reinterpret_pointer_cast<Rpx::Hoi4::Region, Arda::ArdaRegion>(
              modifiableState);

      Elements::borderChild("StateEdit2", [&]() {
        if (longCircuitLogicalOr(
                ImGui::InputInt("Arms Industry", &hoi4Region->armsFactories),
                ImGui::InputInt("Civilian Industry",
                                &hoi4Region->civilianFactories),
                ImGui::InputInt("Naval Industry", &hoi4Region->dockyards),
                ImGui::InputInt("State Category",
                                &hoi4Region->stateCategory))) {
          requireCountryDetails = true;
        }
      });
    }
  }
}

int GUI::showCountryTab(Fwg::Cfg &cfg) {
  if (ImGui::BeginTabItem("Countries")) {
    auto &generator = activeModule->generator;
    if (uiUtils->tabSwitchEvent(true)) {
      uiUtils->updateImage(
          0, generator->visualiseCountries(generator->countryMap));
      uiUtils->updateImage(1, Fwg::Gfx::Bitmap());
    }

    ImGui::Text(
        "Use auto generated country map or drop it in. You may also first "
        "generate a country map, then edit it in the Maps folder, and then "
        "drop it in again.");
    ImGui::Text("You can also drag in a list of countries (in a .txt file) "
                "with the following format: #r;g;b;tag;name;adjective. See "
                "inputs//countryMappings.txt as an example. If no file is "
                "dragged in, this example file is used.");
    ImGui::PushItemWidth(300.0f);
    uiUtils->showHelpTextBox("Countries");
    ImGui::InputInt("Number of countries", &generator->numCountries);
    // ImGui::InputText("Path to country list: ",
    // &generator->countryMappingPath); ImGui::InputText("Path to state list:
    // ", &generator->regionMappingPath);
    ImGui::Checkbox("Draw-borders", &drawBorders);

    if (isRelevantModuleActive("hoi4")) {
      auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
      if (ImGui::Button("Generate state data")) {
        computationFutureBool = runAsync([hoi4Gen, &cfg, this]() {
          hoi4Gen->generateStateSpecifics();
          hoi4Gen->generateStateResources();
          //// generate generic world data
          //Arda::Civilization::generateWorldCivilizations(
          //    hoi4Gen->ardaRegions, hoi4Gen->ardaProvinces, hoi4Gen->civData,
          //    hoi4Gen->scenContinents, activeModule->generator->superRegions);
          Arda::Civilization::generateImportance(hoi4Gen->ardaRegions);
          requireCountryDetails = true;
          return true;
        });
      }
      if (!hoi4Gen->statesInitialised) {
        ImGui::Text("Generate state data first");
      } else {
        if (ImGui::Button("Randomly distribute countries")) {
          computationFutureBool = runAsync([&generator, &cfg, this]() {
            auto countryFactory =
                []() -> std::shared_ptr<Rpx::Hoi4::Hoi4Country> {
              return std::make_shared<Rpx::Hoi4::Hoi4Country>();
            };
            // generate country data
            generator->generateCountries(countryFactory);

            // build hoi4 countries out of basic countries
            generator->mapCountries();
            requireCountryDetails = true;
            uiUtils->resetTexture();
            return true;
          });
        }
        ImGui::SameLine();
        if (requireCountryDetails) {
          ImGui::PushStyleColor(ImGuiCol_Button,
                                ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Red
        }
        if (ImGui::Button("Generate country data")) {
          computationFutureBool = runAsync([hoi4Gen, &cfg, this]() {
            auto countryFactory =
                []() -> std::shared_ptr<Rpx::Hoi4::Hoi4Country> {
              return std::make_shared<Rpx::Hoi4::Hoi4Country>();
            };
            hoi4Gen->generateCountries(countryFactory);

            hoi4Gen->generateLogistics();
            hoi4Gen->generateCountrySpecifics();
            hoi4Gen->generateFocusTrees();
            hoi4Gen->distributeVictoryPoints();
            hoi4Gen->generatePositions();
            requireCountryDetails = false;
            return true;
          });
        }
        if (requireCountryDetails) {
          ImGui::PopStyleColor();
        }
      }
    }

    else if (isRelevantModuleActive("vic3")) {
    } else if (isRelevantModuleActive("eu4")) {
    }

    auto str =
        "Generated countries: " + std::to_string(generator->countries.size());
    ImGui::Text(str.c_str());
    // drag event
    if (triggeredDrag) {
      requireCountryDetails = true;
      triggeredDrag = false;
      if (draggedFile.find(".txt") != std::string::npos) {
        if (draggedFile.find("states.txt") != std::string::npos ||
            draggedFile.find("stateMappings.txt") != std::string::npos) {
          Fwg::Utils::Logging::logLine(
              "Applying state input from file: ",
              Fwg::Utils::userFilter(draggedFile, cfg.username));
          generator->regionMappingPath = draggedFile;
          generator->applyRegionInput();
          requireCountryDetails = true;

        } else if (draggedFile.find("countries.txt") != std::string::npos ||
                   draggedFile.find("countryMappings.txt") !=
                       std::string::npos) {
          Fwg::Utils::Logging::logLine(
              "Applying country input from file: ",
              Fwg::Utils::userFilter(draggedFile, cfg.username));
          generator->countryMappingPath = draggedFile;
          generator->applyCountryInput();
          requireCountryDetails = true;
        } else {
          Fwg::Utils::Logging::logLine(
              "No valid file dragged in, the filename must either be "
              "states.txt, stateMappings.txt, countries.txt or "
              "countryMappings.txt");
        }
      } else {
        computationFutureBool = runAsync([&generator, &cfg, this]() {
          // load countries with correct type, dependent on the gamemodule
          // that is active
          if (isRelevantModuleActive("hoi4")) {
            auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
            hoi4Gen->loadCountries<Rpx::Hoi4::Hoi4Country>(
                draggedFile, generator->countryMappingPath);
          } else if (isRelevantModuleActive("vic3")) {
            generator->loadCountries<Rpx::Vic3::Country>(
                draggedFile, generator->countryMappingPath);
          } else if (isRelevantModuleActive("eu4")) {
            generator->loadCountries<Arda::Country>(
                draggedFile, generator->countryMappingPath);
          }
          Arda::Countries::evaluateCountryNeighbours(
              generator->areaData.regions, generator->ardaRegions,
              generator->countries);
          // build module specific countries out of basic countries
          generator->mapCountries();
          uiUtils->resetTexture();
          return true;
        });
      }
    }

    ImGui::SameLine();
    if (ImGui::Button("Export current countries as image after editing")) {
      Fwg::Gfx::Png::save(generator->visualiseCountries(generator->countryMap),
                          cfg.mapsPath + "countries.png");
      uiUtils->resetTexture();
    }

    countryEdit(generator);

    ImGui::EndTabItem();
    ImGui::PopItemWidth();
  }
  return 0;
}

int GUI::showModuleGeneric(Fwg::Cfg &cfg,
                           std::shared_ptr<Rpx::GenericModule> genericModule) {
  if (!validatedPaths)
    ImGui::BeginDisabled();
  if (genericModule->generator->terrainData.detailedHeightMap.size() &&
      genericModule->generator->climateMap.initialised() &&
      genericModule->generator->provinceMap.initialised() &&
      genericModule->generator->regionMap.initialised()) {
    if (ImGui::Button(
            std::string("Generate " + activeGameConfig.gameName + " mod")
                .c_str())) {

      computationFutureBool =
          runAsyncInitialDisable([genericModule, &cfg, this]() {
            genericModule->generate();
            configuredScenarioGen = true;
            return true;
          });
    }
  }
  ImGui::SameLine();
  if (ImGui::Button(std::string("Generate world + " +
                                activeGameConfig.gameName + " mod in one go")
                        .c_str())) {

    computationFutureBool =
        runAsyncInitialDisable([genericModule, &cfg, this]() {
          genericModule->generator->generateWorld();
          genericModule->generate();
          configuredScenarioGen = true;
          return true;
        });
  }
  if (!validatedPaths)
    ImGui::EndDisabled();
  return 0;
}

int GUI::showStrategicRegionTab(Fwg::Cfg &cfg,
                                std::shared_ptr<Rpx::ModGenerator> &generator) {
  if (ImGui::BeginTabItem("Strategic Regions")) {
    // tab switch setting draw events as accepted
    if (uiUtils->tabSwitchEvent(true)) {
      uiUtils->updateImage(
          0, Arda::Gfx::visualiseStrategicRegions(generator->superRegionMap,
                                                  generator->superRegions));
      uiUtils->updateImage(1, Fwg::Gfx::Bitmap());
    }
    static int selectedStratRegionIndex = 0;
    ImGui::SeparatorText(
        "This generates strategic regions, they cannot be loaded");
    uiUtils->showHelpTextBox("Strategic Regions");
    if (generator->ardaRegions.size()) {
      ImGui::InputFloat(
          "Strategic region factor: ", &generator->superRegionFactor, 0.1f);
      if (ImGui::Button("Generate strategic regions")) {
        // non-country stuff
        computationFutureBool = runAsync([&generator, &cfg, this]() {
          // non-country stuff
          auto factory = []() -> std::shared_ptr<Rpx::StrategicRegion> {
            return std::make_shared<Rpx::StrategicRegion>();
          };
          generator->generateStrategicRegions(factory);
          uiUtils->resetTexture();
          if (activeGameConfig.gameName == "Hearts of Iron IV") {
            auto hoi4Gen =
                std::reinterpret_pointer_cast<Hoi4Gen, Arda::ArdaGen>(
                    activeModule->generator);
            hoi4Gen->generateWeather();
          } else if (activeGameConfig.gameName == "Victoria 3") {
            auto vic3Gen =
                std::reinterpret_pointer_cast<Vic3Gen, Arda::ArdaGen>(
                    activeModule->generator);
            // do stuff
          }
          return true;
        });
      }
      ImGui::SameLine();
      if (ImGui::Button("Visualise current strategic regions")) {
        Arda::Gfx::visualiseStrategicRegions(generator->superRegionMap,
                                             generator->superRegions);
        uiUtils->resetTexture();
      }
      ImGui::Checkbox("Draw strategic regions", &drawBorders);
    }
    // drag event is ignored here
    if (triggeredDrag) {
      Fwg::Utils::Logging::logLine(
          "No loading of strategic regions supported at this time");
      triggeredDrag = false;
    }

    auto &clickEvents = uiUtils->clickEvents;
    if (clickEvents.size()) {
      auto pix = clickEvents.front();
      clickEvents.pop();
      const auto &colour = generator->provinceMap[pix.pixel];
      if (generator->areaData.provinceColourMap.find(colour)) {
        const auto &prov = generator->areaData.provinceColourMap[colour];
        auto &state = generator->ardaRegions[prov->regionID];
        auto &stratRegion = generator->superRegions[state->superRegionID];
        if (drawBorders) {
          auto &rootRegion = generator->superRegions[selectedStratRegionIndex];
          stratRegion->removeRegion(state);
          // if the stratRegion is now empty of regions, remove it
          if (stratRegion->ardaRegions.empty()) {
            generator->superRegions.erase(
                std::remove(generator->superRegions.begin(),
                            generator->superRegions.end(), stratRegion),
                generator->superRegions.end());
            // update all strategic regions IDs
            for (size_t i = 0; i < generator->superRegions.size(); i++) {
              generator->superRegions[i]->ID = i;
              for (auto &region : generator->superRegions[i]->ardaRegions) {
                region->superRegionID = i;
              }
            }
            Fwg::Utils::Logging::logLine(
                "Removed empty strategic region with ID: ", stratRegion->ID);
          }

          rootRegion->addRegion(state);
          uiUtils->updateImage(
              0, Arda::Gfx::visualiseStrategicRegions(generator->superRegionMap,
                                                      generator->superRegions,
                                                      stratRegion->ID));
        } else {
          selectedStratRegionIndex = stratRegion->ID;
        }
      }
    }
    ImGui::EndTabItem();
  }
  return 0;
}

// HOI4
int GUI::showHoi4Configure(Fwg::Cfg &cfg, std::shared_ptr<Hoi4Gen> generator) {
  ImGui::InputDouble("resourceFactor", &generator->resourceFactor, 0.1);
  ImGui::InputDouble("aluminiumFactor", &generator->resources["aluminium"][2],
                     0.1);
  ImGui::InputDouble("chromiumFactor", &generator->resources["chromium"][2],
                     0.1);
  ImGui::InputDouble("oilFactor", &generator->resources["oil"][2], 0.1);
  ImGui::InputDouble("rubberFactor", &generator->resources["rubber"][2], 0.1);
  ImGui::InputDouble("steelFactor", &generator->resources["steel"][2], 0.1);
  ImGui::InputDouble("tungstenFactor", &generator->resources["tungsten"][2],
                     0.1);
  ImGui::InputDouble("baseLightRainChance",
                     &generator->weatherChances["baseLightRainChance"], 0.1);
  ImGui::InputDouble("baseHeavyRainChance",
                     &generator->weatherChances["baseHeavyRainChance"], 0.1);
  ImGui::InputDouble("baseMudChance",
                     &generator->weatherChances["baseMudChance"], 0.1);
  ImGui::InputDouble("baseBlizzardChance",
                     &generator->weatherChances["baseBlizzardChance"], 0.1);
  ImGui::InputDouble("baseSandstormChance",
                     &generator->weatherChances["baseSandstormChance"], 0.1);
  ImGui::InputDouble("baseSnowChance",
                     &generator->weatherChances["baseSnowChance"], 0.1);
  return 0;
}

void GUI::pathWarning(std::exception e) {
  Fwg::Utils::Logging::logLine(
      "Error in writing files: ", e.what(),
      " you probably have misconfigured paths to the mods directory. "
      "According to you, this is located at the following path: ",
      activeModule->pathcfg.gameModPath);
  Fwg::Utils::Logging::logLine("The path to the mod folder is set to ",
                               activeModule->pathcfg.gameModsDirectory,
                               " while the path to the game is set to ",
                               activeModule->pathcfg.gamePath);
  Fwg::Utils::Logging::logLine(
      "Please check if the paths are correct, and if the mod folder is "
      "located in the mods directory of the game. You may now "
      "reconfigure the paths in the first tab and then try to export "
      "again.");
}

int GUI::showHoi4Finalise(Fwg::Cfg &cfg,
                          std::shared_ptr<Rpx::Hoi4::Hoi4Module> hoi4Module) {
  if (ImGui::BeginTabItem("Finalise")) {
    uiUtils->tabSwitchEvent();
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    auto &generator = hoi4Module->hoi4Gen;
    if (generator->superRegions.size() && generator->provinceMap.size() &&
        generator->statesInitialised && !requireCountryDetails) {

      if (ImGui::Button("Export complete mod")) {

        computationFutureBool = runAsync([generator, hoi4Module, &cfg, this]() {
          // now generate hoi4 specific stuff
          try {
            // to recalc if state data was changed after country
            // generation
            generator->evaluateCountries();
            hoi4Module->writeImages();
            hoi4Module->writeTextFiles();
            hoi4Module->writeLocalisation();
            generator->printStatistics();
          } catch (std::exception e) {
            pathWarning(e);
          }
          return true;
        });
      }

      if (ImGui::Button("Export heightmap.bmp")) {
        hoi4Module->formatConverter.dump8BitHeightmap(
            hoi4Module->generator->terrainData.detailedHeightMap,
            hoi4Module->pathcfg.gameModPath + "//map//heightmap", "heightmap");
      }
      if (ImGui::Button("Export world_normal.bmp")) {
        hoi4Module->formatConverter.dumpWorldNormal(
            Fwg::Gfx::displaySobelMap(
                hoi4Module->generator->terrainData.sobelData),
            hoi4Module->pathcfg.gameModPath + "//map//world_normal.bmp", false);
      }
      if (ImGui::Button("Export terrain.bmp")) {
        hoi4Module->formatConverter.dump8BitTerrain(
            hoi4Module->generator->terrainData,
            hoi4Module->generator->climateData, hoi4Module->generator->civLayer,
            hoi4Module->pathcfg.gameModPath + "//map//terrain.bmp", "terrain",
            false);
      }
      if (ImGui::Button("Export provinces.bmp")) {
        Fwg::Gfx::Bmp::save(
            hoi4Module->generator->provinceMap,
            (hoi4Module->pathcfg.gameModPath + ("//map//provinces.bmp"))
                .c_str());
      }
      if (ImGui::Button("Export treemap.bmp")) {
        hoi4Module->formatConverter.dump8BitTrees(
            hoi4Module->generator->terrainData,
            hoi4Module->generator->climateData,
            hoi4Module->pathcfg.gameModPath + "//map//trees.bmp", "trees",
            false);
      }
      if (ImGui::Button("Export colormap_rgb_cityemissivemask_a.dds")) {
        hoi4Module->formatConverter.dumpTerrainColourmap(
            hoi4Module->generator->worldMap, hoi4Module->generator->civLayer,
            hoi4Module->pathcfg.gameModPath,
            "//map//terrain//colormap_rgb_cityemissivemask_a.dds",
            DXGI_FORMAT_B8G8R8A8_UNORM, 2, false);
      }

    } else {
      ImGui::Text("Have strategic regions, initialised states and generated "
                  "country data first before exporting the mod");
    }
    // drag event is ignored here
    if (triggeredDrag) {
      triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }

  return 0;
}

int GUI::showVic3Configure(Fwg::Cfg &cfg, std::shared_ptr<Vic3Gen> generator) {
  // Iterate through each ResConfig in the vector
  auto &resourceConfigs = generator->getResConfigs();
  ImGui::SeparatorText("Configure amount and distribution of resources. Do NOT "
                       "remove the checkmark in the random field, if it is "
                       "preselected. You may add the flag to others.");
  for (size_t i = 0; i < resourceConfigs.size(); ++i) {
    ImGui::Text(resourceConfigs[i].name.c_str());
    ImGui::SameLine();
    // Display and edit the resourcePrevalence field
    ImGui::InputDouble("Resource Prevalence",
                       &resourceConfigs[i].resourcePrevalence);

    ImGui::SameLine();
    // Display and edit the random field
    ImGui::Checkbox("Random", &resourceConfigs[i].random);
  }
  return 0;
}

void GUI::showSplineTab(Fwg::Cfg &cfg,
                        std::shared_ptr<Rpx::Vic3::Module> vic3Module) {
  if (ImGui::BeginTabItem("Splines")) {
    uiUtils->tabSwitchEvent();
    const auto &generator = vic3Module->getGenerator();

    // drag event is ignored here
    if (triggeredDrag) {
      Rpx::Vic3::Splnet spline;
      spline.parseFile(draggedFile);

      spline.writeFile(draggedFile + "overwrite");
      triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }
}

int GUI::showVic3Finalise(Fwg::Cfg &cfg,
                          std::shared_ptr<Rpx::Vic3::Module> vic3Module) {
  if (ImGui::BeginTabItem("Finalise")) {
    uiUtils->tabSwitchEvent();
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    const auto &generator = vic3Module->getGenerator();
    if (generator->superRegions.size()) {
      if (ImGui::Button("Export mod")) {
        computationFutureBool = runAsync([generator, vic3Module, &cfg, this]() {
          // Vic3 specifics:
          generator->distributePops();
          generator->distributeResources();
          generator->mapCountries();
          if (!generator->importData(vic3Module->pathcfg.gamePath +
                                     "//game//")) {
            Fwg::Utils::Logging::logLine(
                "ERROR: Could not import data from game "
                "folder. The export has FAILED. You "
                "must fix the path to the game, then try again");
          } else {
            // handle basic development, tech level, policies,
            generator->generateCountrySpecifics();
            generator->diplomaticRelations();
            generator->createMarkets();
            generator->calculateNeeds();
            generator->distributeBuildings();
            try {
              vic3Module->writeSplnet();
              vic3Module->writeImages();
              vic3Module->writeTextFiles();
            } catch (std::exception e) {
              pathWarning(e);
            }
            generator->printStatistics();
          }
          return true;
        });
      }
    } else {
      ImGui::Text("Generate strategic regions first before exporting the mod");
    }
    // drag event is ignored here
    if (triggeredDrag) {
      triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }

  return 0;
}
