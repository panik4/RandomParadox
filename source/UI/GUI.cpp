#include "UI/GUI.h"
// to track which game was selected for generation
static int selectedGame = 0;
static bool showErrorPopup = false;
static std::string errorLog;
static bool requireCountryDetails = false;

// for state/country/strategic region editing
static bool drawBorders = false;
void GUI::recover() {
  auto &cfg = Fwg::Cfg::Values();
  auto &generator = *activeGenerator;
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
    Fwg::UI::Utils::CreateDeviceGL(window, "RandomParadox 0.10.2", 0, 0);

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
          fwgui->uiContext.draggedFile = (count > 0) ? std::string(paths[count - 1]) : "";
        });

    auto &cfg = Fwg::Cfg::Values();
    auto &io = ImGui::GetIO();
    // rpx related
    this->rpdConf = rpdConfRef;
    this->configSubFolder = configSubFolderRef;
    this->username = usernameRef;
    activeGenerator = std::make_shared<Rpx::Hoi4::Generator>(
        Rpx::Hoi4::Generator(configSubFolder, rpdConf));
    activeGenerator->climateData.addSecondaryColours(Fwg::Parsing::getLines(
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
        "Otherwise try running it through a json validator, e.g. "
        "\"https://jsonlint.com/\" or search for \"json validator\"");
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

bool GUI::isRelevantModuleActive(const std::string &shortName) {
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
  if (ImGui::InputText(label, &path)) {
    Fwg::Parsing::replaceOccurences(path, "\\", "/");
    Fwg::Parsing::replaceOccurences(path, "//", "/");
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
      Fwg::UI::Elements::GridLayout grid(3, 120.0f, 12.0f);

      // Game Selection
      if (grid.AddListBox("Select Game", &selectedGame, gameSelection.data(),
                          static_cast<int>(gameSelection.size()), 4, 350.0f)) {
        if (gameConfigs[selectedGame].gameName == "Hearts of Iron IV") {
          activeGenerator = std::make_shared<Rpx::Hoi4::Generator>(
              Rpx::Hoi4::Generator(configSubFolder, rpdConf));
          activeGenerator->climateData.addSecondaryColours(
              Fwg::Parsing::getLines(Fwg::Cfg::Values().resourcePath +
                                     "hoi4/colourMappings.txt"));
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
    ImGui::PushItemWidth(600.0f);
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
      Fwg::UI::Elements::GridLayout grid(2, 250.0f, 12.0f);

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

void GUI::countryEdit(std::shared_ptr<Arda::ArdaGen> generator) {
  static int selectedStateIndex = 0;
  static std::string drawCountryTag;

  if (!drawBorders) {
    drawCountryTag = "";
  }

  auto &clickEvents = uiContext.drawContext.clickEvents;
  if (clickEvents.size()) {
    auto pix = clickEvents.front();
    clickEvents.pop();
    const auto colour = generator->provinceMap[pix.pixel];
    if (generator->areaData.provinceColourMap.contains(colour)) {
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

      // Country Edit Section
      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 80, 120, 100));
      ImGui::BeginChild("CountryEdit", ImVec2(0, 200), true,
                        ImGuiWindowFlags_None);
      {
        ImGui::SeparatorText("Country Details");

        {
          Fwg::UI::Elements::GridLayout grid(2, 180.0f, 12.0f);

          ImGui::PushItemWidth(180.0f);
          ImGui::AlignTextToFramePadding();
          ImGui::Text("%-*s", 25, "Country Tag");
          ImGui::SameLine();
          if (ImGui::InputText("##tag", &tempTag)) {
            bufferChangedTag = tempTag;
          }
          ImGui::PopItemWidth();

          if (Fwg::UI::Elements::Button("Update Tag", false, ImVec2(120, 0))) {
            if (bufferChangedTag.size() != 3) {
              Fwg::Utils::Logging::logLine("Tag must be 3 characters long");
            } else {
              std::string &oldTag = selectedCountry->tag;
              if (oldTag == bufferChangedTag) {
                Fwg::Utils::Logging::logLine("Tag is the same as the old one");
              } else {
                generator->countries.erase(oldTag);
                selectedCountry->tag = bufferChangedTag;
                generator->countries.insert(
                    {selectedCountry->tag, selectedCountry});
                for (auto &region : selectedCountry->ownedRegions) {
                  region->owner = selectedCountry;
                }
              }
              requireCountryDetails = true;
              generator->visualiseCountries(generator->countryMap,
                                            generator->worldMap);
            }
          }

          ImGui::PushItemWidth(180.0f);
          ImGui::AlignTextToFramePadding();
          ImGui::Text("%-*s", 25, "Country Name");
          ImGui::SameLine();
          ImGui::InputText("##name", &selectedCountry->name);

          ImGui::AlignTextToFramePadding();
          ImGui::Text("%-*s", 25, "Adjective");
          ImGui::SameLine();
          ImGui::InputText("##adj", &selectedCountry->adjective);
          ImGui::PopItemWidth();

          ImVec4 color =
              ImVec4(((float)selectedCountry->colour.getRed()) / 255.0f,
                     ((float)selectedCountry->colour.getGreen()) / 255.0f,
                     ((float)selectedCountry->colour.getBlue()) / 255.0f, 1.0f);

          ImGui::Text("%-*s", 25, "Colour");
          ImGui::SameLine();
          if (ImGui::ColorEdit3("##color", (float *)&color,
                                ImGuiColorEditFlags_NoInputs |
                                    ImGuiColorEditFlags_NoLabel)) {
            selectedCountry->colour = Fwg::Gfx::Colour(
                color.x * 255.0, color.y * 255.0, color.z * 255.0);
            generator->visualiseCountries(generator->countryMap,
                                          generator->worldMap);
            uiContext.imageContext.resetTexture();
          }
        }
      }
      ImGui::EndChild();
      ImGui::PopStyleColor();

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
          generator->visualiseCountries(
              generator->countryMap, generator->worldMap, modifiableState->ID);
          uiContext.imageContext.updateImage(0, generator->countryMap);
        }
      }
    }

    // State Edit Section
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(80, 50, 120, 100));
    ImGui::BeginChild("StateEdit", ImVec2(0, 150), true, ImGuiWindowFlags_None);
    {
      ImGui::SeparatorText("State Details");

      {
        Fwg::UI::Elements::GridLayout grid(2, 180.0f, 12.0f);

        ImGui::PushItemWidth(180.0f);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%-*s", 25, "State Name");
        ImGui::SameLine();
        if (ImGui::InputText("##statename", &modifiableState->name)) {
          requireCountryDetails = true;
        }
        ImGui::PopItemWidth();

        if (modifiableState->owner) {
          grid.AddText("Owner", "%s", modifiableState->owner->tag.c_str());
        }

        if (grid.AddInputDouble("Population", &modifiableState->totalPopulation,
                                0.0, 1000000000.0)) {
          requireCountryDetails = true;
        }
      }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (isRelevantModuleActive("hoi4")) {
      const auto &hoi4Region =
          std::reinterpret_pointer_cast<Rpx::Hoi4::Region, Arda::ArdaRegion>(
              modifiableState);

      ImGui::Spacing();
      ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(120, 80, 50, 100));
      ImGui::BeginChild("StateEditHoi4", ImVec2(0, 150), true,
                        ImGuiWindowFlags_None);
      {
        ImGui::SeparatorText("HOI4 State Specifics");
        // Fwg::UI::Elements::borderChild("StateEdit2", [&]() {
        //   ImGui::PushItemWidth(200.0f);
        //   if (longCircuitLogicalOr(
        //           ImGui::InputInt("Arms Industry",
        //           &hoi4Region->armsFactories), ImGui::InputInt("Civilian
        //           Industry",
        //                           &hoi4Region->civilianFactories),
        //           optionalInput(hoi4Region->isCoastalToOcean(),
        //                         [&] {
        //                           return ImGui::InputInt(
        //                               "Naval Industry",
        //                               &hoi4Region->dockyards);
        //                         }),
        //           ImGui::InputInt("State Category",
        //                           &hoi4Region->stateCategory))) {
        //     requireCountryDetails = true;
        //   }
        //   ImGui::PopItemWidth();
        // });
        {
          Fwg::UI::Elements::GridLayout grid(2, 180.0f, 12.0f);

          if (grid.AddInputInt("Arms Factories", &hoi4Region->armsFactories, 0,
                               100) ||
              grid.AddInputInt("Civilian Factories",
                               &hoi4Region->civilianFactories, 0, 100) ||
              (hoi4Region->isCoastalToOcean() &&
               grid.AddInputInt("Dockyards", &hoi4Region->dockyards, 0, 100)) ||
              grid.AddInputInt("State Category", &hoi4Region->stateCategory, 0,
                               10)) {
            requireCountryDetails = true;
          }
        }
      }
      ImGui::EndChild();
      ImGui::PopStyleColor();
    }
  }
}

void GUI::countryDrag(std::shared_ptr<Arda::ArdaGen> generator) {
  // drag event
  if (uiContext.triggeredDrag) {
    const auto &cfg = Fwg::Cfg::Values();
    requireCountryDetails = true;
    uiContext.triggeredDrag = false;
    if (uiContext.draggedFile.find(".txt") != std::string::npos) {
      if (uiContext.draggedFile.find("states.txt") != std::string::npos ||
          uiContext.draggedFile.find("stateMappings.txt") != std::string::npos) {
        Fwg::Utils::Logging::logLine(
            "Applying state input from file: ",
            Fwg::Utils::userFilter(uiContext.draggedFile, cfg.username));
        generator->regionMappingPath = uiContext.draggedFile;
        generator->applyRegionInput();
        requireCountryDetails = true;

      } else if (uiContext.draggedFile.find("countries.txt") != std::string::npos ||
                 uiContext.draggedFile.find("countryMappings.txt") != std::string::npos) {
        Fwg::Utils::Logging::logLine(
            "Applying country input from file: ",
            Fwg::Utils::userFilter(uiContext.draggedFile, cfg.username));
        activeGenerator->loadCountries(
            activeGenerator->ardaFactories.countryFactory, uiContext.draggedFile);
      } else {
        Fwg::Utils::Logging::logLine(
            "No valid file dragged in, the filename must either be "
            "states.txt, stateMappings.txt, countries.txt or "
            "countryMappings.txt");
      }
    } else {
      uiContext.asyncContext.computationFutureBool =
          uiContext.asyncContext.runAsync([&generator, &cfg, this]() {
            auto evaluationAreas =
                Fwg::UI::Utils::Masks::getLandmaskEvaluationAreas(
                    generator->terrainData.landMask);
            if (cfg.areaInputMode == Fwg::Areas::AreaInputType::SOLID) {
              auto img = Fwg::IO::Reader::readGenericImageWithBorders(
                  uiContext.draggedFile, cfg, evaluationAreas);
              if (img.size()) {
                activeGenerator->loadCountries(
                    generator->ardaFactories.countryFactory, img);
              }
            } else {
              auto image = Fwg::IO::Reader::readGenericImage(uiContext.draggedFile, cfg);

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
            uiContext.imageContext.resetTexture();
            return true;
          });
    }
  }
}

int GUI::showCountryTab(Fwg::Cfg &cfg) {
  if (Fwg::UI::Elements::BeginMainTabItem("Countries")) {
    auto &generator = activeGenerator;
    if (uiContext.tabSwitchEvent(true)) {
      uiContext.imageContext.updateImage(
          0, generator->visualiseCountries(generator->countryMap,
                                           generator->worldMap));
      uiContext.imageContext.updateImage(1, Fwg::Gfx::Image());
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
    ImGui::Checkbox("Draw-borders", &drawBorders);
    ImGui::SameLine();
    ImGui::PushItemWidth(150.0f);
    ImGui::InputInt("Number of countries", &generator->ardaConfig.numCountries);
    Fwg::UI::Areas::areaInputSelector(cfg);

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
      auto exportLocation = Fwg::Cfg::Values().mapsPath + "/areas/";
      if (ImGui::Button(("Export current state of countries and states to " +
                         exportLocation)
                            .c_str())) {
        Arda::Countries::saveCountries(generator->countries, exportLocation);
        Arda::Areas::saveRegions(
            generator->ardaRegions, exportLocation,
            Arda::Gfx::visualiseRegions(generator->ardaRegions));
      }
      if (isRelevantModuleActive("hoi4")) {
        auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
        if (ImGui::Button("Generate state data")) {
          uiContext.asyncContext.computationFutureBool =
              uiContext.asyncContext.runAsync([hoi4Gen, &cfg, this]() {
                hoi4Gen->generateStateSpecifics();
                hoi4Gen->generateStateResources();
                Arda::Civilization::generateImportance(hoi4Gen->ardaRegions);
                requireCountryDetails = true;
                return true;
              });
        }
        if (!hoi4Gen->modData.statesInitialised) {
          ImGui::Text("Generate state data first");
        } else {
          if (ImGui::Button("Randomly distribute countries")) {
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([&generator, &cfg, this]() {
                  auto countryFactory =
                      []() -> std::shared_ptr<Rpx::Hoi4::Hoi4Country> {
                    return std::make_shared<Rpx::Hoi4::Hoi4Country>();
                  };
                  // generate country data
                  generator->generateCountries(countryFactory);

                  // build hoi4 countries out of basic countries
                  generator->mapCountries();
                  requireCountryDetails = true;
                  uiContext.imageContext.resetTexture();
                  return true;
                });
          }
          ImGui::SameLine();
          if (requireCountryDetails) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Red
          }
          if (ImGui::Button("Generate country data")) {
            uiContext.asyncContext.computationFutureBool =
                uiContext.asyncContext.runAsync([hoi4Gen, &cfg, this]() {
                  // generate only country details, no new countries
                  hoi4Gen->generateCountries(nullptr);

                  hoi4Gen->generateLogistics();
                  hoi4Gen->generateCountrySpecifics();
                  // hoi4Gen->generateFocusTrees();
                  hoi4Gen->distributeVictoryPoints();
                  hoi4Gen->generatePositions();
                  hoi4Gen->generateRandomDecisions();
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
      countryDrag(generator);
      countryEdit(generator);
    }

    ImGui::EndTabItem();
    ImGui::PopItemWidth();
  }
  return 0;
}

void GUI::stratRegionEdit(std::shared_ptr<Arda::ArdaGen> generator) {
  static int selectedStratRegionIndex = 0;
  auto &clickEvents = uiContext.drawContext.clickEvents;
  if (clickEvents.size()) {
    auto pix = clickEvents.front();
    clickEvents.pop();
    const auto &colour = generator->provinceMap[pix.pixel];
    if (generator->areaData.provinceColourMap.contains(colour)) {
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
        uiContext.imageContext.updateImage(
            0, Arda::Gfx::visualiseStrategicRegions(generator->superRegionMap,
                                                    generator->superRegions,
                                                    stratRegion->ID));
      } else {
        selectedStratRegionIndex = stratRegion->ID;
      }
    }
  }
}

int GUI::showStrategicRegionTab(Fwg::Cfg &cfg,
                                std::shared_ptr<Rpx::ModGenerator> &generator) {
  if (Fwg::UI::Elements::BeginMainTabItem("Strategic Regions")) {
    if (uiContext.tabSwitchEvent(true)) {
      uiContext.imageContext.updateImage(
          0, Arda::Gfx::visualiseStrategicRegions(generator->superRegionMap,
                                                  generator->superRegions));
      uiContext.imageContext.updateImage(1, Fwg::Gfx::Image());
    }
    uiContext.helpContext.showHelpTextBox("Strategic Regions");
    Fwg::UI::Areas::areaInputSelector(cfg);

    ImGui::SeparatorText("Strategic Region Parameters");

    {
      Fwg::UI::Elements::GridLayout grid(2, 250.0f, 12.0f);

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
                hoi4Gen->generateWeather();
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
              if (cfg.areaInputMode == Fwg::Areas::AreaInputType::SOLID) {
                activeGenerator->loadStrategicRegions(
                    activeGenerator->ardaFactories.superRegionFactory,
                    Fwg::IO::Reader::readGenericImageWithBorders(uiContext.draggedFile,
                                                                 cfg, {}));
              } else {
                auto image =
                    Fwg::IO::Reader::readGenericImage(uiContext.draggedFile, cfg);
                Fwg::Gfx::Filter::colouriseAreaBorderInputByBordersOnly(image,
                                                                        {});
                Fwg::Gfx::Filter::fillBlackPixelsByArea(image, {});
                activeGenerator->loadStrategicRegions(
                    activeGenerator->ardaFactories.superRegionFactory, image);
              }

              if (activeGameConfig.gameName == "Hearts of Iron IV") {
                auto hoi4Gen =
                    std::reinterpret_pointer_cast<Hoi4Gen, Arda::ArdaGen>(
                        activeGenerator);
                hoi4Gen->generateWeather();
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
             requireCountryDetails),
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
                generator->evaluateCountries();
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
    Fwg::UI::Elements::GridLayout grid(3, 180.0f, 12.0f);

    for (size_t i = 0; i < resourceConfigs.size(); ++i) {
      ImGui::PushID(i);

      ImGui::Text("%-*s", 20, resourceConfigs[i].name.c_str());
      ImGui::SameLine();

      ImGui::PushItemWidth(120.0f);
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
