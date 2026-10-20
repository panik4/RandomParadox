#include "UI/GUI.h"
#include <algorithm>
// to track which game was selected for generation
static int selectedGame = 0;
static bool showErrorPopup = false;
static std::string errorLog;
static bool requireSoftCountryDetails = false;
static bool requireHardCountryDetails = false;
static bool useSimulationOutput = false;

void GUI::recover() {
  auto &cfg = Fwg::Cfg::Values();
  auto &generator = *activeGenerator;
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
    showGeneric(cfg, *activeGenerator);
    if (!validatedPaths)
      ImGui::EndDisabled();
    ImGui::SameLine();
    showModuleGeneric(cfg);
    // end genericwrapper
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
  if (redoDevelopment || redoPopulation || redoTopography || redoCulture ||
      redoLocations) {
    ImGui::BeginDisabled();
  }
  if (activeGameConfig.gameName == "Hearts of Iron IV") {
    auto hoi4Gen = std::reinterpret_pointer_cast<Hoi4Gen, Rpx::ModGenerator>(
        activeGenerator);
    showStrategicRegionTab(cfg, activeGenerator);
    showCountryTab(cfg);
    showHoi4Finalise(cfg);
  } else if (activeGameConfig.gameName == "Victoria 3") {
    auto vic3Gen = std::reinterpret_pointer_cast<Vic3Gen, Rpx::ModGenerator>(
        activeGenerator);
    showStrategicRegionTab(cfg, activeGenerator);
    showCountryTab(cfg);
    showNavmeshTab(cfg);
    showVic3Finalise(cfg);
  } else if (activeGameConfig.gameName == "Europa Universalis V") {
    auto eu5Gen =
        std::reinterpret_pointer_cast<Rpx::Eu5::Generator, Rpx::ModGenerator>(
            activeGenerator);
  }
  if (redoDevelopment || redoPopulation || redoTopography || redoCulture ||
      redoLocations) {
    ImGui::EndDisabled();
  }
}

bool setWindowIcon(GLFWwindow *window, const std::string &path) {
  int width, height, channels;
  unsigned char *pixels =
      stbi_load(path.c_str(), &width, &height, &channels, 4);
  if (!pixels) {
    return false;
  }

  GLFWimage icon;
  icon.width = width;
  icon.height = height;
  icon.pixels = pixels;

  glfwSetWindowIcon(window, 1, &icon);

  stbi_image_free(pixels);
  return true;
}

int GUI::shiny(const pt::ptree &rpdConfRef,
               const std::string &configSubFolderRef,
               const std::string &usernameRef) {

  try {
    Fwg::UI::Utils::CreateDeviceGL(window, "RandomParadox 0.11.1", 0, 0);

    setWindowIcon(window, Fwg::Cfg::Values().workingDirectory +
                              "resources/worldMap.png");
    Fwg::UI::Utils::setupImGuiContextAndStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    glfwSetWindowUserPointer(window, this);
    glfwSetDropCallback(
        window, [](GLFWwindow *win, int count, const char **paths) {
          auto *fwgui = reinterpret_cast<GUI *>(glfwGetWindowUserPointer(win));
          fwgui->uiContext.triggeredDrag = (count > 0);
          fwgui->uiContext.draggedFile =
              (count > 0) ? std::string(paths[count - 1]) : "";
        });

    auto &cfg = Fwg::Cfg::Values();
    auto &io = ImGui::GetIO();
    // rpx related
    this->rpdConf = rpdConfRef;
    this->configSubFolder = configSubFolderRef;
    this->username = usernameRef;
    activeGenerator = std::make_shared<Rpx::Hoi4::Generator>(
        Rpx::Hoi4::Generator(configSubFolder, rpdConf));
    cfg.climateConfig.addSecondaryColours(Fwg::Parsing::getLines(
        Fwg::Cfg::Values().resourcePath + "hoi4/colourMappings.txt"));
    initGameConfigs();

    activeGenerator->configure(cfg);

    init(cfg, *activeGenerator);

    while (!glfwWindowShouldClose(window)) {
      uiContext.triggeredDrag = false;
      glfwPollEvents();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();

      ImGui::NewFrame();
      // Apply automatic UI scaling based on window resolution
      applyAutoUIScale();
      {
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
        ImGui::Begin("RandomParadox");
        {
          // Push style to remove padding so we can place button in top-right
          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

          // Calculate top-right position for button
          ImVec2 buttonSize = ImVec2(50, 30); // width x height
          ImVec2 windowSize = ImGui::GetWindowSize();
          ImGui::SetCursorPos(ImVec2(windowSize.x - buttonSize.x - 10, 20));

          if (ImGui::Button("Exit", buttonSize)) {
            glfwSetWindowShouldClose(window, true); // trigger close
          }

          ImGui::PopStyleVar();
        }
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
                          ImVec2(ImGui::GetContentRegionAvail().x *
                                     uiContext.layoutContext.leftColumnWidth,
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

            if (Fwg::UI::Elements::BeginMainTabBar("Steps")) {
              // Disable all inputs if computation is running
              if (uiContext.asyncContext.computationRunning) {
                ImGui::BeginDisabled();
              }
              // reset to default before tabs can again overwrite it, if custom
              // width needed
              uiContext.layoutContext.leftColumnWidth = 0.4f;
              showConfigure(cfg);
              if (!validatedPaths)
                ImGui::BeginDisabled();
              // if (cfg.debugLevel == 9) {
              //   showModLoader(cfg);
              // }
              defaultTabs(cfg, *activeGenerator);
              automapAreas();
              showCivilizationTab(cfg);
              gameSpecificTabs(cfg);
              auto ardaGen =
                  std::static_pointer_cast<Arda::ArdaGen>(activeGenerator);
              overview(cfg);
              if (!validatedPaths)
                ImGui::EndDisabled();
              // Re-enable inputs if computation is running
              if (uiContext.asyncContext.computationRunning &&
                  !uiContext.asyncContext.computationStarted) {
                ImGui::EndDisabled();
              }
              // Check if the computation is done
              if (uiContext.asyncContext.computationRunning &&
                  uiContext.asyncContext.computationFutureBool.wait_for(
                      std::chrono::seconds(0)) == std::future_status::ready) {
                uiContext.asyncContext.computationRunning = false;
                uiContext.imageContext.resetTexture();
              }

              if (uiContext.asyncContext.computationRunning) {
                uiContext.asyncContext.computationStarted = false;
                ImGui::Text("Working, please be patient");
              } else {
                ImGui::Text("Ready!");
              }

              Fwg::UI::Elements::EndMainTabBar();
            }

            ImGui::PopStyleColor();
          }
          // ends SettingsContent
          ImGui::EndChild();
          // Draw a frame around the child region
          ImVec2 childMin = ImGui::GetItemRectMin();
          ImVec2 childMax = ImGui::GetItemRectMax();
          ImGui::GetWindowDrawList()->AddRect(
              childMin, childMax, IM_COL32(100, 90, 180, 255), 0.0f, 0, 2.0f);

          genericWrapper();
          logWrapper();
        }
        // ends LeftContent
        ImGui::EndChild();
        ImGui::SameLine();
        imageWrapper(io);
        ImGui::End();

        if (uiContext.helpContext.showExtendedHelp) {
          uiContext.helpContext.showAdvancedTextBox();
        }
      }

      // Render
      ImGui::Render();
      int display_w, display_h;
      glfwGetFramebufferSize(window, &display_w, &display_h);
      glViewport(0, 0, display_w, display_h);
      glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Fwg::UI::Utils::CleanupDeviceGL(window);
    return 0;
  } catch (std::exception &e) {
    Fwg::Utils::Logging::logLine("Error in GUI startup: ", e.what());
    return -1;
  }
}

void GUI::loadConfigs() {
  configSubfolders.clear();
  for (const auto &entry : std::filesystem::directory_iterator(
           Fwg::Cfg::Values().workingDirectory + "configs")) {
    if (entry.is_directory() && !entry.path().string().contains("heightmap")) {
      configSubfolders.push_back(entry.path().string());
      Fwg::Utils::Logging::logLine(entry);
    }
  }
}

void GUI::loadGameConfig(Fwg::Cfg &cfg) {
  namespace pt = boost::property_tree;
  pt::ptree hoi4Conf;
  try {
    // Read the basic settings
    std::ifstream f(activeConfig + "/Hearts of Iron IVModule.json");
    std::stringstream buffer;
    if (!f.good())
      Fwg::Utils::Logging::logLine("Config could not be loaded");
    buffer << f.rdbuf();
    Fwg::Parsing::replaceInStringStream(buffer, "//", "/");

    pt::read_json(buffer, hoi4Conf);
  } catch (std::exception &e) {
    Fwg::Utils::Logging::logLine("Incorrect config \"RandomParadox.json\"");
    Fwg::Utils::Logging::logLine("You can try fixing it yourself. Error is: ",
                                 e.what());
    Fwg::Utils::Logging::logLine(
        "Otherwise try running it through a json validator");
  }
}
// hardcoded init of some game configs
void GUI::initGameConfigs() {
  gameConfigs.push_back({"Hearts of Iron IV", "hoi4"});
  gameConfigs.push_back({"Victoria 3", "vic3"});
  gameConfigs.push_back({"Europa Universalis IV", "eu4"});
  gameConfigs.push_back({"Europa Universalis V", "eu5"});
  activeGameConfig = gameConfigs[0];
}

bool GUI::validatePaths() {
  validatedPaths = Rpx::Utils::autoLocateGameFolder(
      activeGenerator->pathcfg.gamePath, activeGameConfig.gameName,
      activeGenerator->pathcfg.gameSubPath);
  if (validatedPaths)
    validatedPaths =
        Rpx::Utils::validateGameModFolder(activeGenerator->pathcfg);
  if (validatedPaths)
    validatedPaths = Rpx::Utils::validateModFolder(activeGenerator->pathcfg);
  activeGenerator->initImageExporter();
  activeGenerator->nData = Rpx::NameGeneration::prepare(
      Fwg::Cfg::Values().resourcePath + "names",
      activeGenerator->pathcfg.gamePath, activeGenerator->gameType);
  return validatedPaths;
}

int GUI::showModuleGeneric(Fwg::Cfg &cfg) {
  if (!validatedPaths)
    ImGui::BeginDisabled();
  if (activeGenerator->terrainData.detailedHeightMap.size() &&
      activeGenerator->climateMap.initialised() &&
      activeGenerator->provinceMap.initialised() &&
      activeGenerator->regionMap.initialised()) {
    if (ImGui::Button(
            std::string("Generate " + activeGameConfig.gameName + " mod")
                .c_str())) {

      uiContext.asyncContext.computationFutureBool =
          uiContext.asyncContext.runAsyncInitialDisable([&cfg, this]() {
            activeGenerator->generate();
            redoTopography = false;
            redoDevelopment = false;
            redoPopulation = false;
            redoCulture = false;
            redoLocations = false;
            return true;
          });
    }
  }
  ImGui::SameLine();
  if (ImGui::Button(std::string("Generate world + " +
                                activeGameConfig.gameName + " mod in one go")
                        .c_str())) {

    uiContext.asyncContext.computationFutureBool =
        uiContext.asyncContext.runAsyncInitialDisable([&cfg, this]() {
          activeGenerator->generateWorld();
          activeGenerator->generate();
          return true;
        });
  }
  if (!validatedPaths)
    ImGui::EndDisabled();
  return 0;
}

void GUI::pathWarning(std::exception e) {
  Fwg::Utils::Logging::logLine(
      "Error in writing files: ", e.what(),
      " you probably have misconfigured paths to the mods directory. "
      "According to you, this is located at the following path: ",
      activeGenerator->pathcfg.gameModPath);
  Fwg::Utils::Logging::logLine("The path to the mod folder is set to ",
                               activeGenerator->pathcfg.gameModsDirectory,
                               " while the path to the game is set to ",
                               activeGenerator->pathcfg.gamePath);
  Fwg::Utils::Logging::logLine(
      "Please check if the paths are correct, and if the mod folder is "
      "located in the mods directory of the game. You may now "
      "reconfigure the paths in the first tab and then try to export "
      "again.");
}

bool GUI::isRelevantModuleActive(const std::string &shortName) const {
  return activeGameConfig.gameShortName == shortName;
}

int GUI::showConfigure(Fwg::Cfg &cfg) {

  if (Fwg::UI::Elements::BeginMainTabItem("Configure")) {
    uiContext.layoutContext.leftColumnWidth = 1.0f;
    uiContext.helpContext.showHelpTextBox("Configure");
    if (Fwg::UI::Elements::BeginSubTabBar("Config tabs", 0.0f)) {
      showRpdxConfigure(cfg);
      showFwgConfigure(cfg);
      Fwg::UI::Elements::EndSubTabBar();
    }
    ImGui::EndTabItem();
  }
  return 0;
}

// Helper function for path input with normalization
static bool PathInput(const char *label, std::string &path,
                      bool validate = true) {
  ImGui::InputText(label, &path);
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    Rpx::Utils::sanitizePath(path);
    return validate;
  }
  return false;
}

int GUI::showRpdxConfigure(Fwg::Cfg &cfg) {
  static int item_current = 0;

  if (Fwg::UI::Elements::BeginSubTabItem("RandomParadox Configuration")) {
    uiContext.tabSwitchEvent();

    if (!loadedConfigs) {
      loadedConfigs = true;
      loadConfigs();
      activeConfig = configSubfolders[item_current];
      Rpx::Utils::autoLocateGameFolder(activeGenerator->pathcfg.gamePath,
                                       activeGameConfig.gameName,
                                       activeGenerator->pathcfg.gameSubPath);
      Rpx::Utils::autoLocateGameModFolder(activeGameConfig.gameName,
                                          activeGenerator->pathcfg);
      validatePaths();
    }

    ImGui::SeparatorText("Game and Configuration Selection");

    // Prepare data for grid layout
    std::vector<const char *> gameSelection;
    for (auto &gameConfig : gameConfigs) {
      gameSelection.push_back(gameConfig.gameName.c_str());
    }

    std::vector<const char *> items;
    for (auto &item : configSubfolders)
      items.push_back(item.c_str());

    // Game and Config selection side-by-side using GridLayout
    {
      Fwg::UI::Elements::GridLayout grid(3, scaled(120.0f), scaled(12.0f));

      // Game Selection
      if (grid.AddListBox("Select Game", &selectedGame, gameSelection.data(),
                          static_cast<int>(gameSelection.size()), 4, 350.0f)) {
        if (gameConfigs[selectedGame].gameName == "Hearts of Iron IV") {
          activeGenerator = std::make_shared<Rpx::Hoi4::Generator>(
              Rpx::Hoi4::Generator(configSubFolder, rpdConf));
          cfg.climateConfig.addSecondaryColours(Fwg::Parsing::getLines(
              Fwg::Cfg::Values().resourcePath + "hoi4/colourMappings.txt"));
        } else if (gameConfigs[selectedGame].gameName ==
                   "Europa Universalis IV") {
          activeGenerator = std::make_shared<Rpx::Eu4::Generator>(
              Rpx::Eu4::Generator(configSubFolder, rpdConf));
        } else if (gameConfigs[selectedGame].gameName == "Victoria 3") {
          activeGenerator = std::make_shared<Rpx::Vic3::Generator>(
              Rpx::Vic3::Generator(configSubFolder, rpdConf));
        } else if (gameConfigs[selectedGame].gameName ==
                   "Europa Universalis V") {
          activeGenerator = std::make_shared<Rpx::Eu5::Generator>(
              Rpx::Eu5::Generator(configSubFolder, rpdConf));
        }
        activeGameConfig = gameConfigs[selectedGame];
        Rpx::Utils::autoLocateGameFolder(activeGenerator->pathcfg.gamePath,
                                         activeGameConfig.gameName,
                                         activeGenerator->pathcfg.gameSubPath);
        Rpx::Utils::autoLocateGameModFolder(activeGameConfig.gameName,
                                            activeGenerator->pathcfg);
        validatedPaths = false;
        validatePaths();
        activeGenerator->configure(cfg);
      }

      // Config Preset Selection
      // if (grid.AddListBox("Config Presets", &item_current, items.data(),
      //                    static_cast<int>(items.size()), 4, 450.0f)) {
      //  activeConfig = items[item_current];
      //  Fwg::Utils::Logging::logLine("Switched to ", activeConfig,
      //                               "/FastWorldGenerator.json");
      //  cfg.readConfig(activeConfig);
      //  loadGameConfig(cfg);
      //  activeGenerator->configure(cfg);
      //}
    }

    ardaGen = activeGenerator;

    ImGui::Spacing();
    ImGui::SeparatorText("Path Configuration");

    // Path inputs with automatic normalization
    bool needsValidation = false;
    ImGui::PushItemWidth(scaled(600.0f));
    needsValidation |= PathInput("Mod Name", activeGenerator->pathcfg.modName);
    needsValidation |=
        PathInput("Game Path", activeGenerator->pathcfg.gamePath);
    needsValidation |=
        PathInput("Mods Directory", activeGenerator->pathcfg.gameModsDirectory);
    needsValidation |=
        PathInput("Mod Path", activeGenerator->pathcfg.gameModPath, false);
    ImGui::PopItemWidth();

    if (needsValidation) {
      validatePaths();
    }

    ImGui::Spacing();

    if (Fwg::UI::Elements::Button("Find Game Files", false, ImVec2(180, 0))) {
      Rpx::Utils::autoLocateGameFolder(activeGenerator->pathcfg.gamePath,
                                       activeGameConfig.gameName,
                                       activeGenerator->pathcfg.gameSubPath);
    }
    ImGui::SameLine();
    if (Fwg::UI::Elements::Button("Find Mods Folder", false, ImVec2(180, 0))) {
      Rpx::Utils::autoLocateGameModFolder(activeGameConfig.gameName,
                                          activeGenerator->pathcfg);
    }
    ImGui::SameLine();
    if (Fwg::UI::Elements::ImportantStepButton("Validate Paths",
                                               ImVec2(180, 0))) {
      validatePaths();
    }

    ImGui::Spacing();
    ImGui::SeparatorText("World Configuration");

    {
      Fwg::UI::Elements::GridLayout grid(2, scaled(250.0f), scaled(12.0f));

      if (grid.AddInputDouble(
              "Population Factor",
              &activeGenerator->ardaConfig.worldPopulationFactor, 0.0, 10.0)) {
        activeGenerator->ardaConfig.calculateTargetWorldPopulation();
      }
      grid.AddText("Target Population", "%.1f Mio",
                   ardaGen->ardaConfig.targetWorldPopulation / 1000'000.0);

      if (grid.AddInputDouble("Industry Factor",
                              &activeGenerator->ardaConfig.worldIndustryFactor,
                              0.0, 10.0)) {
        activeGenerator->ardaConfig.calculateTargetWorldGdp();
      }
      grid.AddText("Target GDP", "%.2f Trillion",
                   ardaGen->ardaConfig.targetWorldGdp / 1000'000'000'000.0);
    }

    ImGui::Spacing();

    if (isRelevantModuleActive("hoi4")) {
      auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
      Rpx::UI::Hoi4::showHoi4Configure(cfg, hoi4Gen);
    } else if (isRelevantModuleActive("vic3")) {
      auto vic3Gen = getGeneratorPointer<Vic3Gen>();
      showVic3Configure(cfg, vic3Gen);
    } else if (isRelevantModuleActive("eu4")) {
      auto eu4Gen = getGeneratorPointer<Eu4Gen>();
    }

    ImGui::EndTabItem();
  }

  cfg.climateMappingPath = Fwg::Cfg::Values().resourcePath + "" +
                           activeGameConfig.gameShortName +
                           "/climateMapping.txt ";
  return 0;
}
void GUI::showModLoader(Fwg::Cfg &cfg) {
  if (Fwg::UI::Elements::BeginMainTabItem("Modloader")) {
    if (uiContext.triggeredDrag) {
      // auto hoi4Module =
      //     std::reinterpret_pointer_cast<Rpx::Hoi4::Hoi4Module,
      //                                   Rpx::GenericModule>(genericModule);
      // hoi4Module->readHoi(uiContext.draggedFile);
      uiContext.triggeredDrag = false;
      uiContext.imageContext.resetTexture();
    }
    ImGui::EndTabItem();
  }
}

void GUI::processMapEditorSelection() {
  auto &clickEvents = uiContext.drawContext.clickEvents;
  if (clickEvents.empty()) {
    return;
  }

  const auto click = clickEvents.front();
  clickEvents.pop();
  if (click.pixel < 0 ||
      click.pixel >= static_cast<int>(activeGenerator->provinceMap.size())) {
    return;
  }

  const auto provinceColour = activeGenerator->provinceMap[click.pixel];
  auto &provinceColourMap = activeGenerator->areaData.provinceColourMap;
  if (!provinceColourMap.contains(provinceColour)) {
    return;
  }

  const auto &province = provinceColourMap[provinceColour];
  if (!province) {
    return;
  }
  switch (mapEditorState.activeLayer) {
  case Rpx::UI::MapEditorLayer::Countries:
  case Rpx::UI::MapEditorLayer::States:
    if (province->regionID >= 0 &&
        province->regionID <
            static_cast<int>(activeGenerator->ardaRegions.size())) {
      const auto &state = activeGenerator->ardaRegions[province->regionID];
      if (state) {
        mapEditorState.selectedStateID = state->ID;
        if (!mapEditorState.drawBorders && state->owner) {
          mapEditorState.selectedCountryTag = state->owner->tag;
        }
      }
    }
    break;
  case Rpx::UI::MapEditorLayer::Provinces:
    mapEditorState.selectedProvinceID = province->ID;
    break;
  case Rpx::UI::MapEditorLayer::Continents:
    if (province->continent) {
      mapEditorState.selectedContinentID = -1;
      for (const auto &continent : activeGenerator->ardaContinents) {
        if (continent && continent->ID == province->continent->ID) {
          mapEditorState.selectedContinentID = continent->ID;
          break;
        }
      }
    } else {
      mapEditorState.selectedContinentID = -1;
    }
    break;
  case Rpx::UI::MapEditorLayer::Locations:
    mapEditorState.selectedLocationID = -1;
    for (const auto &region : activeGenerator->areaData.regions) {
      if (!region) {
        continue;
      }
      const auto location = std::ranges::find_if(
          region->locations, [pixel = click.pixel](const auto &candidate) {
            return candidate && std::ranges::find(candidate->pixels, pixel) !=
                                    candidate->pixels.end();
          });
      if (location != region->locations.end()) {
        mapEditorState.selectedLocationID = (*location)->ID;
        break;
      }
    }
    break;
  case Rpx::UI::MapEditorLayer::StrategicRegions:
    break;
  }
}

std::shared_ptr<Arda::ArdaRegion> GUI::getSelectedMapEditorState() const {
  const auto state = std::ranges::find_if(
      activeGenerator->ardaRegions, [this](const auto &candidate) {
        return candidate->ID == mapEditorState.selectedStateID;
      });
  return state == activeGenerator->ardaRegions.end() ? nullptr : *state;
}

std::shared_ptr<Arda::ArdaProvince> GUI::getSelectedMapEditorProvince() const {
  const auto province = std::ranges::find_if(
      activeGenerator->ardaProvinces, [this](const auto &candidate) {
        return candidate && candidate->ID == mapEditorState.selectedProvinceID;
      });
  return province == activeGenerator->ardaProvinces.end() ? nullptr : *province;
}

std::shared_ptr<Fwg::Civilization::Location>
GUI::getSelectedMapEditorLocation() const {
  for (const auto &region : activeGenerator->areaData.regions) {
    if (!region) {
      continue;
    }
    const auto location =
        std::ranges::find_if(region->locations, [this](const auto &candidate) {
          return candidate &&
                 candidate->ID == mapEditorState.selectedLocationID;
        });
    if (location != region->locations.end()) {
      return *location;
    }
  }
  return nullptr;
}

std::shared_ptr<Arda::ArdaContinent>
GUI::getSelectedMapEditorContinent() const {
  const auto continent = std::ranges::find_if(
      activeGenerator->ardaContinents, [this](const auto &candidate) {
        return candidate && candidate->ID == mapEditorState.selectedContinentID;
      });
  return continent == activeGenerator->ardaContinents.end() ? nullptr
                                                            : *continent;
}

std::shared_ptr<Arda::Country> GUI::getSelectedMapEditorCountry(
    const std::shared_ptr<Arda::ArdaRegion> &state) const {
  if (!mapEditorState.selectedCountryTag.empty()) {
    const auto country =
        activeGenerator->countries.find(mapEditorState.selectedCountryTag);
    if (country != activeGenerator->countries.end()) {
      return country->second;
    }
  }
  return state ? state->owner : nullptr;
}

const Rpx::UI::MapEditorModule *GUI::getMapEditorModule() {
  if (activeGameConfig.gameShortName == "hoi4") {
    static const Rpx::UI::Hoi4::MapEditorModule hoi4Module;
    return &hoi4Module;
  }
  return nullptr;
}

void GUI::showGenericCountryEditor(
    const std::shared_ptr<Arda::Country> &country) {
  std::string updatedTag = country->tag;
  static std::string bufferedTag;

  ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
  if (!ImGui::CollapsingHeader("Country Editing")) {
    return;
  }

  ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 80, 120, 100));
  ImGui::BeginChild("CountryEdit", ImVec2(0, 500), true, ImGuiWindowFlags_None);
  ImGui::SeparatorText("Country Details");

  Fwg::UI::Elements::GridLayout grid(2, scaled(180.0f), scaled(12.0f),
                                     scaled(180.0f));
  if (const auto *module = getMapEditorModule()) {
    module->showCountryAggregates(country, grid);
  }

  if (grid.AddInputText("Country Tag", &updatedTag)) {
    bufferedTag = updatedTag;
  }

  if (Fwg::UI::Elements::Button("Update Tag", false, ImVec2(120, 0))) {
    if (bufferedTag.size() != 3) {
      Fwg::Utils::Logging::logLine("Tag must be 3 characters long");
    } else if (country->tag == bufferedTag) {
      Fwg::Utils::Logging::logLine("Tag is the same as the old one");
    } else {
      const auto oldTag = country->tag;
      activeGenerator->countries.erase(oldTag);
      country->tag = bufferedTag;
      activeGenerator->countries.insert({country->tag, country});
      mapEditorState.selectedCountryTag = country->tag;
      mapEditorState.borderPaintCountryTag = country->tag;
      for (auto &region : country->ownedRegions) {
        region->owner = country;
      }
    }
    activeGenerator->visualiseCountries(activeGenerator->countryMap,
                                        activeGenerator->worldMap);
    refreshMapEditorVisualization();
  }

  grid.AddInputText("Country Name", &country->name);
  grid.AddInputText("Adjective", &country->adjective);

  if (const auto *module = getMapEditorModule()) {
    if (module->showCountryEditor(country, grid)) {
      requireHardCountryDetails = true;
    }
  }

  ImVec4 color = ImVec4(country->colour.getRed() / 255.0f,
                        country->colour.getGreen() / 255.0f,
                        country->colour.getBlue() / 255.0f, 1.0f);
  ImGui::Text("%-*s", 25, "Colour");
  ImGui::SameLine();
  if (ImGui::ColorEdit3("##color", &color.x,
                        ImGuiColorEditFlags_NoInputs |
                            ImGuiColorEditFlags_NoLabel)) {
    country->colour =
        Fwg::Gfx::Colour(color.x * 255.0, color.y * 255.0, color.z * 255.0);
    activeGenerator->visualiseCountries(activeGenerator->countryMap,
                                        activeGenerator->worldMap);
    refreshMapEditorVisualization();
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
}

void GUI::showGenericStateEditor(
    const std::shared_ptr<Arda::ArdaRegion> &state) {
  ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
  if (!ImGui::CollapsingHeader("State Editing")) {
    return;
  }

  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(80, 50, 120, 100));
  ImGui::BeginChild("StateEdit", ImVec2(0, 300), true, ImGuiWindowFlags_None);
  ImGui::SeparatorText("State Details");
  Fwg::UI::Elements::GridLayout grid(2, scaled(180.0f), scaled(12.0f),
                                     scaled(180.0f));
  grid.AddInputText("State Name", &state->name);
  if (state->owner) {
    grid.AddText("Owner", "%s", state->owner->tag.c_str());
  }
  if (grid.AddInputDouble("Population", &state->totalPopulation, 0.0,
                          1000000000.0)) {
    requireHardCountryDetails = true;
  }
  if (const auto *module = getMapEditorModule()) {
    if (module->showStateEditor(state, grid)) {
      requireHardCountryDetails = true;
    }
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
}

void GUI::showGenericAreaNameEditor(
    const std::shared_ptr<Fwg::Areas::Area> &area, const char *label) {
  if (!area) {
    ImGui::Text("Select a %s on the map to edit its name.", label);
    return;
  }

  ImGui::SeparatorText(label);
  ImGui::Text("ID: %d", area->ID);
  ImGui::SetNextItemWidth(scaled(260.0f));
  ImGui::InputText((std::string("Name##") + label).c_str(), &area->name);
}

void GUI::showGenericContinentSummary(
    const std::shared_ptr<Arda::ArdaContinent> &continent,
    Fwg::UI::Elements::GridLayout &grid) {
  if (!continent) {
    return;
  }

  double population = 0.0;
  double gdp = 0.0;
  std::map<std::string, double> resources;
  for (const auto &region : continent->ardaRegions) {
    if (!region) {
      continue;
    }
    population += region->totalPopulation;
    gdp += region->gdp;
    for (const auto &[name, resource] : region->resources) {
      resources[name] += resource.amount;
    }
  }

  grid.AddText("Continent Summary", "%s", "");
  grid.AddText("Regions", "%zu", continent->ardaRegions.size());
  grid.AddText("Provinces", "%zu", continent->ardaProvinces.size());
  grid.AddText("Population", "%.0f", population);
  grid.AddText("GDP", "%.2f", gdp);
  grid.AddText("Economic Activity Share", "%.2f%%",
               continent->worldEconomicActivityShare * 100.0);
  if (!resources.empty()) {
    grid.AddText("Resources", "%s", "");
    for (const auto &[name, amount] : resources) {
      grid.AddText(name.c_str(), "%.0f", amount);
    }
  }
}

void GUI::applyCountryBorderPaint(
    const std::shared_ptr<Arda::ArdaRegion> &state) {
  if (!mapEditorState.drawBorders ||
      mapEditorState.borderPaintCountryTag.empty()) {
    return;
  }
  const auto country =
      activeGenerator->countries.find(mapEditorState.borderPaintCountryTag);
  if (country == activeGenerator->countries.end() || state->isSea() ||
      state->owner == country->second) {
    return;
  }
  if (state->owner) {
    state->owner->removeRegion(state);
  }
  state->owner = country->second;
  country->second->addRegion(state);
  requireHardCountryDetails = true;
  activeGenerator->visualiseCountries(activeGenerator->countryMap,
                                      activeGenerator->worldMap, state->ID);
  refreshMapEditorVisualization();
}

void GUI::refreshMapEditorVisualization() {
  Arda::UI::VisualLayerPreset preset;
  switch (mapEditorState.activeLayer) {
  case Rpx::UI::MapEditorLayer::Countries:
    preset = Arda::UI::VisualLayerPreset::Countries;
    break;
  case Rpx::UI::MapEditorLayer::States:
    preset = Arda::UI::VisualLayerPreset::States;
    break;
  case Rpx::UI::MapEditorLayer::Provinces:
    preset = Arda::UI::VisualLayerPreset::Provinces;
    break;
  case Rpx::UI::MapEditorLayer::Locations:
    preset = Arda::UI::VisualLayerPreset::Locations;
    break;
  case Rpx::UI::MapEditorLayer::Continents:
    preset = Arda::UI::VisualLayerPreset::Continents;
    break;
  case Rpx::UI::MapEditorLayer::StrategicRegions:
    preset = Arda::UI::VisualLayerPreset::StrategicRegions;
    break;
  }
  uiContext.imageContext.updateImage(
      0,
      Arda::UI::renderVisualLayers(
          *activeGenerator, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
          activeGenerator->areaGenerationContext(Fwg::Cfg::Values()),
          Arda::UI::createVisualLayerPreset(preset)));
  uiContext.imageContext.updateImage(1, Fwg::Gfx::Image());
}

void GUI::refreshStrategicRegionVisualization() {
  uiContext.imageContext.updateImage(
      0,
      Arda::UI::renderVisualLayers(
          *activeGenerator, Fwg::Cfg::Values().width, Fwg::Cfg::Values().height,
          activeGenerator->areaGenerationContext(Fwg::Cfg::Values()),
          Arda::UI::createVisualLayerPreset(
              Arda::UI::VisualLayerPreset::StrategicRegions),
          selectedStrategicRegionID));
  uiContext.imageContext.updateImage(1, Fwg::Gfx::Image());
}

void GUI::countryEdit() {
  processMapEditorSelection();
  const auto state = getSelectedMapEditorState();
  if (!state) {
    return;
  }

  const auto country = getSelectedMapEditorCountry(state);
  if (country) {
    showGenericCountryEditor(country);
  }
  applyCountryBorderPaint(state);
  showGenericStateEditor(state);
}

void GUI::stateEdit() {
  processMapEditorSelection();
  const auto state = getSelectedMapEditorState();
  if (!state) {
    return;
  }
  showGenericStateEditor(state);
}

void GUI::showMapEditorLayer() {
  switch (mapEditorState.activeLayer) {
  case Rpx::UI::MapEditorLayer::Countries:
    countryEdit();
    break;
  case Rpx::UI::MapEditorLayer::States:
    stateEdit();
    break;
  case Rpx::UI::MapEditorLayer::Provinces:
    processMapEditorSelection();
    showGenericAreaNameEditor(getSelectedMapEditorProvince(), "Province");
    break;
  case Rpx::UI::MapEditorLayer::Locations:
    processMapEditorSelection();
    if (ImGui::Button("Re-derive location names from languages")) {
      for (const auto &region : activeGenerator->ardaRegions) {
        if (!region) {
          continue;
        }
        const auto culture = region->getPrimaryCulture();
        if (!culture || !culture->language) {
          continue;
        }
        for (const auto &location : region->locations) {
          if (location) {
            location->name = culture->language->generateAreaName("");
          }
        }
      }
    }
    showGenericAreaNameEditor(getSelectedMapEditorLocation(), "Location");
    break;
  case Rpx::UI::MapEditorLayer::Continents:
    processMapEditorSelection();
    if (const auto continent = getSelectedMapEditorContinent()) {
      showGenericAreaNameEditor(continent, "Continent");
      Fwg::UI::Elements::GridLayout grid(2, scaled(180.0f), scaled(12.0f),
                                         scaled(180.0f));
      showGenericContinentSummary(continent, grid);
      if (const auto *module = getMapEditorModule()) {
        module->showContinentAggregates(continent, grid);
      }
    } else {
      showGenericAreaNameEditor(nullptr, "Continent");
    }
    break;
  case Rpx::UI::MapEditorLayer::StrategicRegions:
    ImGui::TextUnformatted(
        "Strategic-region editing is available in the Strategic Regions tab.");
    break;
  }
}

void GUI::countryDrag() {
  // drag event
  if (uiContext.triggeredDrag) {
    const auto &cfg = Fwg::Cfg::Values();
    requireHardCountryDetails = true;
    uiContext.triggeredDrag = false;
    if (uiContext.draggedFile.find(".txt") != std::string::npos) {
      if (uiContext.draggedFile.find("states.txt") != std::string::npos ||
          uiContext.draggedFile.find("stateMappings.txt") !=
              std::string::npos) {
        Fwg::Utils::Logging::logLine(
            "Applying state input from file: ",
            Fwg::Utils::userFilter(uiContext.draggedFile, cfg.username));
        activeGenerator->regionMappingPath = uiContext.draggedFile;
        activeGenerator->applyRegionInput();
        requireHardCountryDetails = true;

      } else if (uiContext.draggedFile.find("countries.txt") !=
                     std::string::npos ||
                 uiContext.draggedFile.find("countryMappings.txt") !=
                     std::string::npos) {
        Fwg::Utils::Logging::logLine(
            "Applying country input from file: ",
            Fwg::Utils::userFilter(uiContext.draggedFile, cfg.username));
        activeGenerator->loadCountries(
            activeGenerator->ardaFactories.countryFactory,
            uiContext.draggedFile);
      } else {
        Fwg::Utils::Logging::logLine(
            "No valid file dragged in, the filename must either be "
            "states.txt, stateMappings.txt, countries.txt or "
            "countryMappings.txt");
      }
    } else {
      uiContext.asyncContext.computationFutureBool =
          uiContext.asyncContext.runAsync([&cfg, this]() {
            auto evaluationAreas =
                Fwg::UI::Utils::Masks::getLandmaskEvaluationAreas(
                    activeGenerator->terrainData.landMask);
            if (cfg.areaConfig.common.inputMode ==
                Fwg::Areas::AreaInputType::SOLID) {
              auto img = Fwg::IO::Reader::readGenericImageWithBorders(
                  uiContext.draggedFile, cfg, evaluationAreas);
              if (img.size()) {
                activeGenerator->loadCountries(
                    activeGenerator->ardaFactories.countryFactory, img);
              }
            } else {
              auto image =
                  Fwg::IO::Reader::readGenericImage(uiContext.draggedFile, cfg);

              if (image.size()) {
                // detect all areas, give them unique colours
                Fwg::Gfx::Filter::colouriseAreaBorderInputByBordersOnly(
                    image, evaluationAreas);

                // now that we have modified the input image with colours
                // filling the areas between borders, we can remove the borders
                Fwg::Gfx::Filter::fillBlackPixelsByArea(image, evaluationAreas);
                activeGenerator->loadCountries(
                    activeGenerator->ardaFactories.countryFactory, image);
              }
            }
            refreshMapEditorVisualization();
            return true;
          });
    }
  }
}

int GUI::showCountryTab(Fwg::Cfg &cfg) {
  if (Fwg::UI::Elements::BeginMainTabItem("Countries")) {
    if (uiContext.tabSwitchEvent(true)) {
      refreshMapEditorVisualization();
    }

    uiContext.helpContext.showHelpTextBox("Countries");
    ImGui::Text(
        "Use auto generated country map or drop it in. You may also first "
        "generate a country map, then edit it in the Maps folder, and then "
        "drop it in again.");
    ImGui::Text("You can also drag in a list of countries (in a .txt file) "
                "with the following format: #r;g;b;tag;name;adjective. See "
                "inputs/countryMappings.txt as an example. If no file is "
                "dragged in, this example file is used.");
    const char *editorLayers[] = {"Countries",  "States",
                                  "Provinces",  "Locations",
                                  "Continents", "Strategic Regions"};
    auto selectedLayer = static_cast<int>(mapEditorState.activeLayer);
    if (ImGui::Combo("Editor layer", &selectedLayer, editorLayers,
                     IM_ARRAYSIZE(editorLayers))) {
      mapEditorState.activeLayer =
          static_cast<Rpx::UI::MapEditorLayer>(selectedLayer);
      switch (mapEditorState.activeLayer) {
      case Rpx::UI::MapEditorLayer::Provinces:
        mapEditorState.selectedProvinceID = -1;
        break;
      case Rpx::UI::MapEditorLayer::Locations:
        mapEditorState.selectedLocationID = -1;
        break;
      case Rpx::UI::MapEditorLayer::Continents:
        mapEditorState.selectedContinentID = -1;
        break;
      default:
        break;
      }
      refreshMapEditorVisualization();
    }
    ImGui::Checkbox("Draw-borders", &mapEditorState.drawBorders);
    ImGui::SameLine();
    ImGui::PushItemWidth(scaled(150.0f));
    ImGui::InputInt("Number of countries",
                    &activeGenerator->ardaConfig.numCountries);
    Fwg::UI::Areas::areaInputSelector(cfg);

    auto guard = Rpx::UI::RpxPrerequisiteChecker::require(
        {Fwg::UI::PrerequisiteChecker::climate(activeGenerator->climateData),
         Fwg::UI::PrerequisiteChecker::landforms(activeGenerator->terrainData),
         Fwg::UI::PrerequisiteChecker::habitability(
             activeGenerator->climateData),
         Fwg::UI::PrerequisiteChecker::superSegments(activeGenerator->areaData),
         Fwg::UI::PrerequisiteChecker::segments(activeGenerator->areaData),
         Fwg::UI::PrerequisiteChecker::provinces(activeGenerator->areaData),
         Fwg::UI::PrerequisiteChecker::regions(activeGenerator->areaData),
         Fwg::UI::PrerequisiteChecker::continents(activeGenerator->areaData),
         Arda::UI::ArdaPrerequisiteChecker::ardaRegions(*activeGenerator),
         Arda::UI::ArdaPrerequisiteChecker::ardaProvinces(*activeGenerator),
         Arda::UI::ArdaPrerequisiteChecker::ardaContinents(*activeGenerator)});
    if (guard.ready()) {
      auto exportLocation = Fwg::Cfg::Values().mapsPath + "areas/";
      if (ImGui::Button(("Export current state of countries and states to " +
                         exportLocation)
                            .c_str())) {
        Arda::Countries::saveCountries(activeGenerator->countries,
                                       exportLocation);
        Arda::Areas::saveRegions(
            activeGenerator->ardaRegions, exportLocation,
            Arda::Gfx::visualiseRegions(activeGenerator->ardaRegions));
      }
      if (isRelevantModuleActive("hoi4")) {
        auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
        if (ImGui::Button("Generate state data")) {
          uiContext.asyncContext.computationFutureBool =
              uiContext.asyncContext.runAsync([hoi4Gen, &cfg, this]() {
                hoi4Gen->generateStateSpecifics();
                hoi4Gen->generateStateResources();
                Arda::Civilization::generateImportance(hoi4Gen->ardaRegions);
                requireHardCountryDetails = true;
                return true;
              });
        }
        if (!hoi4Gen->modData.statesInitialised) {
          ImGui::Text("Generate state data first");
        } else {
          if (ImGui::Button("Randomly distribute countries")) {
            cfg.randomSeed = true;
            cfg.reRandomize();
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([hoi4Gen, &cfg, this]() {
                  auto countryFactory =
                      []() -> std::shared_ptr<Rpx::Hoi4::Hoi4Country> {
                    return std::make_shared<Rpx::Hoi4::Hoi4Country>();
                  };
                  // generate country data
                  hoi4Gen->generateCountries(countryFactory);

                  // build hoi4 countries out of basic countries
                  hoi4Gen->mapCountries();
                  requireHardCountryDetails = true;
                  uiContext.imageContext.resetTexture();
                  return true;
                });
          }
          ImGui::SameLine();
          if (ImGui::Button("Derive countries from simulation")) {
            cfg.randomSeed = true;
            cfg.reRandomize();
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([hoi4Gen, &cfg, this]() {
                  auto countryFactory =
                      []() -> std::shared_ptr<Rpx::Hoi4::Hoi4Country> {
                    return std::make_shared<Rpx::Hoi4::Hoi4Country>();
                  };
                  // generate country data
                  hoi4Gen->deriveCountries(countryFactory);

                  // build hoi4 countries out of basic countries
                  hoi4Gen->mapCountries();
                  hoi4Gen->deriveCountrySpecificsFromSimulation();
                  requireHardCountryDetails = false;
                  requireSoftCountryDetails = false;
                  uiContext.imageContext.resetTexture();
                  return true;
                });
          }
          ImGui::SameLine();
          if (requireHardCountryDetails) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Red
          }

          if (ImGui::Button("Generate country territory data")) {
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([hoi4Gen, &cfg, this]() {
                  // generate only country details, no new countries
                  hoi4Gen->generateCountries(nullptr);
                  // Hard is politics agnostic, but dependent on owned territory
                  hoi4Gen->generateHardCountrySpecifics();
                  requireHardCountryDetails = false;
                  return true;
                });
          }
          if (ImGui::Button("Recalculate country tech and forces")) {
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([hoi4Gen]() {
                  hoi4Gen->recalculateCountryMilitaryData();
                  return true;
                });
          }
          ImGui::SameLine();
          if (ImGui::Button("Regenerate country forces")) {
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([hoi4Gen]() {
                  hoi4Gen->generateCountryForces();
                  return true;
                });
          }
          if (requireHardCountryDetails) {
            ImGui::PopStyleColor();
          }
          if (requireHardCountryDetails) {
            ImGui::BeginDisabled();
          }
          if (requireSoftCountryDetails) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Red
          }
          if (ImGui::Button("Generate country political data")) {
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([hoi4Gen, &cfg, this]() {
                  // soft follows hard, as soft includes balancing and politics.
                  requireSoftCountryDetails = false;
                  hoi4Gen->generateSoftCountryDetails();
                  return true;
                });
          }
          if (requireSoftCountryDetails) {
            ImGui::PopStyleColor();
          }
          if (requireHardCountryDetails) {
            ImGui::EndDisabled();
          }
        }
      }

      else if (isRelevantModuleActive("vic3")) {
      } else if (isRelevantModuleActive("eu4")) {
      }

      auto str = "Generated countries: " +
                 std::to_string(activeGenerator->countries.size());
      ImGui::Text(str.c_str());
      countryDrag();
      showMapEditorLayer();
    }

    ImGui::EndTabItem();
    ImGui::PopItemWidth();
  }
  return 0;
}

void GUI::stratRegionEdit(std::shared_ptr<Arda::ArdaGen> generator) {
  auto &clickEvents = uiContext.drawContext.clickEvents;
  if (clickEvents.size()) {
    auto pix = clickEvents.front();
    clickEvents.pop();
    if (pix.pixel < 0 ||
        pix.pixel >= static_cast<int>(generator->provinceMap.size())) {
      return;
    }
    const auto &colour = generator->provinceMap[pix.pixel];
    if (generator->areaData.provinceColourMap.contains(colour)) {
      const auto &prov = generator->areaData.provinceColourMap[colour];
      if (prov->regionID < 0 ||
          prov->regionID >= static_cast<int>(generator->ardaRegions.size())) {
        return;
      }
      auto &state = generator->ardaRegions[prov->regionID];
      if (state->superRegionID < 0 ||
          state->superRegionID >=
              static_cast<int>(generator->superRegions.size())) {
        return;
      }
      auto &stratRegion = generator->superRegions[state->superRegionID];
      if (strategicRegionBorderPaint) {
        if (selectedStrategicRegionID < 0 ||
            selectedStrategicRegionID >=
                static_cast<int>(generator->superRegions.size())) {
          return;
        }
        auto &rootRegion = generator->superRegions[selectedStrategicRegionID];
        stratRegion->removeRegion(state);
        // if the stratRegion is now empty of regions, remove it
        if (stratRegion->ardaRegions.empty()) {
          generator->superRegions.erase(
              std::remove(generator->superRegions.begin(),
                          generator->superRegions.end(), stratRegion),
              generator->superRegions.end());
          // update all strategic regions IDs
          for (size_t i = 0; i < generator->superRegions.size(); i++) {
            generator->superRegions[i]->ID = static_cast<int>(i);
            for (auto &region : generator->superRegions[i]->ardaRegions) {
              region->superRegionID = i;
            }
          }
          Fwg::Utils::Logging::logLine(
              "Removed empty strategic region with ID: ", stratRegion->ID);
        }
        if (rootRegion != nullptr) {
          rootRegion->addRegion(state);
        }
        refreshStrategicRegionVisualization();
      } else {
        selectedStrategicRegionID = stratRegion->ID;
        refreshStrategicRegionVisualization();
      }
    }
  }
}

int GUI::showStrategicRegionTab(Fwg::Cfg &cfg,
                                std::shared_ptr<Rpx::ModGenerator> &generator) {
  if (Fwg::UI::Elements::BeginMainTabItem("Strategic Regions")) {
    if (uiContext.tabSwitchEvent(true)) {
      selectedStrategicRegionID = -1;
      refreshStrategicRegionVisualization();
    }
    uiContext.helpContext.showHelpTextBox("Strategic Regions");
    Fwg::UI::Areas::areaInputSelector(cfg);
    ImGui::Checkbox("Draw-borders", &strategicRegionBorderPaint);

    ImGui::SeparatorText("Strategic Region Parameters");

    {
      Fwg::UI::Elements::GridLayout grid(2, scaled(250.0f), scaled(12.0f));

      grid.AddInputFloat("Region Factor",
                         &generator->ardaConfig.superRegionFactor, 0.0f, 10.0f);
      grid.AddInputFloat("Min Distance Factor",
                         &generator->ardaConfig.superRegionMinDistanceFactor,
                         0.0f, 10.0f);
    }

    ImGui::Spacing();
    auto guard = Rpx::UI::RpxPrerequisiteChecker::require(
        {Fwg::UI::PrerequisiteChecker::climate(ardaGen->climateData),
         Fwg::UI::PrerequisiteChecker::landforms(ardaGen->terrainData),
         Fwg::UI::PrerequisiteChecker::habitability(ardaGen->climateData),
         Fwg::UI::PrerequisiteChecker::superSegments(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::segments(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::provinces(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::regions(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::continents(ardaGen->areaData),
         Arda::UI::ArdaPrerequisiteChecker::ardaRegions(*ardaGen),
         Arda::UI::ArdaPrerequisiteChecker::ardaProvinces(*ardaGen),
         Arda::UI::ArdaPrerequisiteChecker::ardaContinents(*ardaGen)});

    if (guard.ready()) {
      if (Fwg::UI::Elements::Button("Generate Template", false,
                                    ImVec2(200, 0))) {
        Arda::Gfx::generateStrategicRegionTemplate(
            generator->areaData.provinces, generator->areaData.regions);
      }

      ImGui::SameLine();

      if (Fwg::UI::Elements::ImportantStepButton("Generate Strategic Regions",
                                                 ImVec2(250, 0))) {
        uiContext.asyncContext.computationFutureBool =
            uiContext.asyncContext.runAsync([&generator, &cfg, this]() {
              generator->generateStrategicRegions(
                  activeGenerator->ardaFactories.superRegionFactory);
              uiContext.imageContext.resetTexture();
              if (activeGameConfig.gameName == "Hearts of Iron IV") {
                auto hoi4Gen =
                    std::reinterpret_pointer_cast<Hoi4Gen, Arda::ArdaGen>(
                        activeGenerator);
              } else if (activeGameConfig.gameName == "Victoria 3") {
                auto vic3Gen =
                    std::reinterpret_pointer_cast<Vic3Gen, Arda::ArdaGen>(
                        activeGenerator);
              }
              return true;
            });
      }

      if (uiContext.triggeredDrag) {
        uiContext.asyncContext.computationFutureBool =
            uiContext.asyncContext.runAsync([&generator, &cfg, this]() {
              if (cfg.areaConfig.common.inputMode ==
                  Fwg::Areas::AreaInputType::SOLID) {
                activeGenerator->loadStrategicRegions(
                    activeGenerator->ardaFactories.superRegionFactory,
                    Fwg::IO::Reader::readGenericImageWithBorders(
                        uiContext.draggedFile, cfg, {}));
              } else {
                auto image = Fwg::IO::Reader::readGenericImage(
                    uiContext.draggedFile, cfg);
                Fwg::Gfx::Filter::colouriseAreaBorderInputByBordersOnly(image,
                                                                        {});
                Fwg::Gfx::Filter::fillBlackPixelsByArea(image, {});
                activeGenerator->loadStrategicRegions(
                    activeGenerator->ardaFactories.superRegionFactory, image);
              }
              uiContext.triggeredDrag = false;
              uiContext.imageContext.resetTexture();
              return true;
            });
      }

      stratRegionEdit(generator);
    }
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showHoi4Finalise(Fwg::Cfg &cfg) {
  if (Fwg::UI::Elements::BeginMainTabItem("Finalise")) {
    uiContext.tabSwitchEvent();
    uiContext.helpContext.showHelpTextBox("Finalise");
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    auto generator = getGeneratorPointer<Rpx::Hoi4::Generator>();

    auto guard = Rpx::UI::RpxPrerequisiteChecker::require(
        {Fwg::UI::PrerequisiteChecker::climate(ardaGen->climateData),
         Fwg::UI::PrerequisiteChecker::landforms(ardaGen->terrainData),
         Fwg::UI::PrerequisiteChecker::habitability(ardaGen->climateData),
         Fwg::UI::PrerequisiteChecker::superSegments(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::segments(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::provinces(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::regions(ardaGen->areaData),
         Fwg::UI::PrerequisiteChecker::continents(ardaGen->areaData),
         Arda::UI::ArdaPrerequisiteChecker::ardaRegions(*ardaGen),
         Arda::UI::ArdaPrerequisiteChecker::ardaProvinces(*ardaGen),
         Arda::UI::ArdaPrerequisiteChecker::ardaContinents(*ardaGen),
         Rpx::UI::RpxPrerequisiteChecker::strategicRegions(*generator),
         Rpx::UI::RpxPrerequisiteChecker::countryDetailsReady(
             requireHardCountryDetails),
         Rpx::UI::RpxPrerequisiteChecker::hoi4StatesInitialised(*generator)});

    if (guard.ready()) {
      ImGui::SeparatorText(
          "Export everything into a (hopefully) functional mod.");
      static bool writeScenarioDetails = true;

      ImGui::Checkbox("Write scenario details", &writeScenarioDetails);
      if (ImGui::Button("Export complete mod")) {
        uiContext.asyncContext.computationFutureBool =
            uiContext.asyncContext.runAsync([generator, &cfg, this]() {
              // now generate hoi4 specific stuff
              try {
                if (validatedPaths) {
                  activeGenerator->createPaths();
                }
                // to recalc if state data was changed after country
                // generation
                generator->finaliseData();
                generator->writeImages();
                generator->writeTextFiles(writeScenarioDetails);
                generator->writeLocalisation();
                generator->printStatistics();
              } catch (std::exception &e) {
                pathWarning(e);
              }
              return true;
            });
      }

      ImGui::SeparatorText(
          "Separate exports, if you know what you're doing...");
      if (ImGui::Button("Export heightmap.bmp")) {
        generator->getImageExporter().dump8BitHeightmap(
            generator->terrainData.detailedHeightMap,
            generator->pathcfg.gameModPath + "map/heightmap", "heightmap");
      }
      if (ImGui::Button("Export world_normal.bmp")) {
        generator->getImageExporter().dumpWorldNormal(
            Fwg::Gfx::displaySobelMap(generator->terrainData.sobelData),
            generator->pathcfg.gameModPath + "map/world_normal.bmp", false);
      }
      if (ImGui::Button("Export terrain.bmp")) {
        generator->getImageExporter().dump8BitTerrain(
            generator->terrainData, generator->climateData,
            generator->ardaData.civLayer,
            generator->pathcfg.gameModPath + "map/terrain.bmp", "terrain",
            false);
      }
      if (ImGui::Button("Export provinces.bmp")) {
        Fwg::Gfx::Bmp::save(
            generator->provinceMap,
            (generator->pathcfg.gameModPath + ("map/provinces.bmp")).c_str());
      }
      if (ImGui::Button("Export treemap.bmp")) {
        generator->getImageExporter().dump8BitTrees(
            generator->terrainData, generator->climateData,
            generator->pathcfg.gameModPath + "map/trees.bmp", "trees", false);
      }
      if (ImGui::Button("Export colormap_rgb_cityemissivemask_a.dds")) {
        generator->getImageExporter().dumpTerrainColourmap(
            generator->worldMap, generator->ardaData.civLayer,
            generator->pathcfg.gameModPath,
            "map/terrain/colormap_rgb_cityemissivemask_a.dds",
            gli::format::FORMAT_BGR8_UNORM_PACK32, 2, false);
      }
    }
    // drag event is ignored here
    if (uiContext.triggeredDrag) {
      uiContext.triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }

  return 0;
}

int GUI::showVic3Configure(Fwg::Cfg &cfg, std::shared_ptr<Vic3Gen> generator) {
  ImGui::SeparatorText("Resource Configuration");
  ImGui::TextWrapped(
      "Configure amount and distribution of resources. "
      "Do NOT remove the checkmark in the random field if preselected.");

  auto &resourceConfigs = generator->getResConfigs();

  {
    Fwg::UI::Elements::GridLayout grid(3, scaled(180.0f), scaled(12.0f));

    for (size_t i = 0; i < resourceConfigs.size(); ++i) {
      ImGui::PushID(i);

      ImGui::Text("%-*s", 20, resourceConfigs[i].name.c_str());
      ImGui::SameLine();

      ImGui::PushItemWidth(scaled(120.0f));
      ImGui::InputDouble("##prevalence",
                         &resourceConfigs[i].resourcePrevalence);
      ImGui::PopItemWidth();

      ImGui::SameLine();
      ImGui::Checkbox("##random", &resourceConfigs[i].random);

      ImGui::PopID();

      grid.NextRow();
    }
  }

  return 0;
}

void GUI::showSplineTab(Fwg::Cfg &cfg) {
  if (Fwg::UI::Elements::BeginMainTabItem("Splines")) {
    uiContext.tabSwitchEvent();

    // drag event is ignored here
    if (uiContext.triggeredDrag) {
      Rpx::Vic3::Splnet spline;
      spline.parseFile(uiContext.draggedFile);

      spline.writeFile(uiContext.draggedFile + "overwrite");
      uiContext.triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }
}

int GUI::showVic3Finalise(Fwg::Cfg &cfg) {
  if (Fwg::UI::Elements::BeginMainTabItem("Finalise")) {
    uiContext.tabSwitchEvent();
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    const auto &generator = getGeneratorPointer<Rpx::Vic3::Generator>();
    if (generator->superRegions.size()) {
      if (ImGui::Button("Export mod")) {
        uiContext.asyncContext.computationFutureBool =
            uiContext.asyncContext.runAsync([generator, &cfg, this]() {
              // Vic3 specifics:
              generator->distributeResources();
              generator->mapCountries();
              if (!generator->importData(generator->pathcfg.gamePath +
                                         "/game/")) {
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
                  generator->writeSplnet();
                  generator->writeImages();
                  generator->writeTextFiles(true);
                } catch (std::exception &e) {
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
    if (uiContext.triggeredDrag) {
      uiContext.triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }

  return 0;
}

void GUI::showEu5Finalise(Fwg::Cfg &cfg) {
  if (Fwg::UI::Elements::BeginMainTabItem("Finalise")) {
    uiContext.tabSwitchEvent();
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    const auto &generator = getGeneratorPointer<Rpx::Eu5::Generator>();
    if (generator->superRegions.size()) {
      if (ImGui::Button("Export mod")) {
        uiContext.asyncContext.computationFutureBool =
            uiContext.asyncContext.runAsync([generator, &cfg, this]() {
              generator->writeImages();
              return true;
            });
      }
    } else {
      ImGui::Text("Generate strategic regions first before exporting the mod");
    }
    // drag event is ignored here
    if (uiContext.triggeredDrag) {
      uiContext.triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }
}
