#include "UI/GUI.h"
// Data
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
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
GUI::GUI() : fwgUI() {}

int GUI::shiny(const pt::ptree &rpdConf, const std::string &configSubFolder,
               const std::string &username) {

  //  Create application window
  //  ImGui_ImplWin32_EnableDpiAwareness();
  WNDCLASSEXW wc = {sizeof(wc),
                    CS_CLASSDC,
                    WndProc,
                    0L,
                    0L,
                    GetModuleHandle(nullptr),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    L"RandomParadox",
                    nullptr};
  HICON hIcon = (HICON)LoadImage(NULL, "resources//worldMap.ico", IMAGE_ICON, 0,
                                 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
  if (hIcon) {
    // Icon loaded successfully, set it to the window class
    wc.hIcon = hIcon;
  } else {
    // Icon failed to load, handle error
    DWORD error = GetLastError();
    // handle error...
  }

  HWND consoleWindow = GetConsoleWindow();

  SendMessage(consoleWindow, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
  SendMessage(consoleWindow, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
  ::RegisterClassExW(&wc);
  HWND hwnd =
      uiUtils->createAndConfigureWindow(wc, wc.lpszClassName, L"RandomParadox");
  // Initialize Direct3D
  if (!CreateDeviceD3D(hwnd)) {
    CleanupDeviceD3D();
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 1;
  }

  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

  // Setup Dear ImGui context
  uiUtils->setupImGuiContextAndStyle();
  auto &io = ImGui::GetIO();
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  uiUtils->setupImGuiBackends(hwnd, g_pd3dDevice, g_pd3dDeviceContext);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  auto &cfg = Fwg::Cfg::Values();
  // Main loop
  bool done = false;
  //--- prior to main loop:
  DragAcceptFiles(hwnd, TRUE);
  curtexture = nullptr;
  this->rpdConf = rpdConf;
  this->configSubFolder = configSubFolder;
  this->username = username;
  activeModule = std::make_shared<Scenario::Hoi4::Hoi4Module>(
      Scenario::Hoi4::Hoi4Module(rpdConf, configSubFolder, username, false));
  initGameConfigs();
  this->pathconfig = activeModule->pathcfg;
  frequency = cfg.overallFrequencyModifier;
  log = std::make_shared<std::stringstream>();
  *log << Fwg::Utils::Logging::Logger::logInstance.getFullLog();
  Fwg::Utils::Logging::Logger::logInstance.attachStream(log);
  while (!done) {
    // reset dragging all the time in case it wasn't handled in a tab on purpose
    triggeredDrag = false;
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32
    // backend.
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        done = true;
      else if (msg.message == WM_DROPFILES) {
        HDROP hDrop = reinterpret_cast<HDROP>(msg.wParam);

        // extract files here
        std::vector<std::string> files;
        char filename[MAX_PATH];

        UINT count = DragQueryFileA(hDrop, -1, NULL, 0);
        for (UINT i = 0; i < count; ++i) {
          if (DragQueryFileA(hDrop, i, filename, MAX_PATH)) {
            files.push_back(filename);
            // Fwg::Utils::Logging::logLine("Loaded file ", filename);
          }
        }
        draggedFile = files.back();
        triggeredDrag = true;
        DragFinish(hDrop);
      }
    }
    if (done)
      break;

    // Handle window resize (we don't resize directly in the WM_SIZE handler)
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
      CleanupRenderTarget();
      g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                                  DXGI_FORMAT_UNKNOWN, 0);
      g_ResizeWidth = g_ResizeHeight = 0;
      CreateRenderTarget();
    }

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
      if (!validatedPaths)
        ImGui::BeginDisabled();
      showGeneric(cfg, *activeModule->generator, &curtexture);
      if (!validatedPaths)
        ImGui::EndDisabled();
      ImGui::SameLine();
      showModuleGeneric(cfg, activeModule);
      ImGui::SeparatorText(
          "Different Steps of the generation, usually go from left to right");

      if (uiUtils->actTxs[1] == UIUtils::ActiveTexture::NONE &&
          secondaryTexture != nullptr) {
        uiUtils->freeTexture(&secondaryTexture);
      }
      if (ImGui::BeginTabBar("Steps", ImGuiTabBarFlags_None)) {
        showConfigure(cfg, activeModule);
        if (!validatedPaths)
          ImGui::BeginDisabled();
        // showModLoader(cfg, activeModule);
        showHeightmapTab(cfg, *activeModule->generator, &curtexture);
        showLandTab(cfg, *activeModule->generator);
        showNormalMapTab(cfg, *activeModule->generator, &curtexture);
        showContinentTab(cfg, *activeModule->generator, &curtexture);
        showClimateInputTab(cfg, *activeModule->generator, &curtexture);
        showClimateOverview(cfg, *activeModule->generator, &curtexture);
        showDensityTab(cfg, *activeModule->generator, &curtexture);
        showBordersTab(cfg, *activeModule->generator);
        showAreasTab(cfg, *activeModule->generator);
        showCivilizationTab(cfg, *activeModule->generator);
        showScenarioTab(cfg, activeModule);
        if (!configuredScenarioGen) {
          ImGui::BeginDisabled();
        }
        showCountryTab(cfg, &curtexture);
        if (activeGameConfig.gameName == "Hearts of Iron IV") {
          auto hoi4Gen =
              std::reinterpret_pointer_cast<Hoi4Gen, Scenario::Generator>(
                  activeModule->generator);
          showStrategicRegionTab(cfg, hoi4Gen);
          showHoi4Finalise(
              cfg, std::reinterpret_pointer_cast<Scenario::Hoi4::Hoi4Module,
                                                 Scenario::GenericModule>(
                       activeModule));
        }
        if (!configuredScenarioGen) {
          ImGui::EndDisabled();
        }
        if (!validatedPaths)
          ImGui::EndDisabled();

        ImGui::EndTabBar();
      }
      static ImVec2 cursorPos;

      float modif = 1.0 - (secondaryTexture != nullptr) * 0.5;
      if (w > 0 && h > 0) {
        float aspectRatio = (float)w / (float)h;
        auto scale =
            std::min<float>((ImGui::GetContentRegionAvail().y) / h,
                            (ImGui::GetContentRegionAvail().x) * modif / w);
        auto texWidth = w * scale;
        auto texHeight = h * scale;

        // Handle zooming
        if (io.KeyCtrl) {
          zoom += io.MouseWheel * 0.1f;
        }

        if (curtexture != nullptr &&
            uiUtils->actTxs[0] != UIUtils::ActiveTexture::NONE) {
          // Create a child window with scrollbars
          ImGui::BeginChild("Image", ImVec2(texWidth, texHeight), false,
                            ImGuiWindowFlags_HorizontalScrollbar |
                                ImGuiWindowFlags_AlwaysVerticalScrollbar);

          ImGui::Image((void *)curtexture,
                       ImVec2(texWidth * zoom, texHeight * zoom));
          static auto delta = ImGui::GetIO().MouseDelta.x;

          if (io.KeyCtrl && io.MouseWheel) {
            // Get the mouse position relative to the image
            ImVec2 mouse_pos = ImGui::GetMousePos();
            ImVec2 image_pos = ImGui::GetItemRectMin();
            auto itemsize = ImGui::GetItemRectSize();
            ImVec2 mouse_pos_relative =
                ImVec2(mouse_pos.x - image_pos.x, mouse_pos.y - image_pos.y);
            // Calculate the pixel position in the texture
            float pixel_x = ((mouse_pos_relative.x / itemsize.x));
            float pixel_y = ((mouse_pos_relative.y / itemsize.y));
            ImGui::SetScrollHereY(std::clamp(pixel_y, 0.0f, 1.0f));
            ImGui::SetScrollHereX(std::clamp(pixel_x, 0.0f, 1.0f));
          }

          // Handle dragging
          if (io.KeyCtrl && ImGui::IsMouseDragging(0, 0.0f)) {
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(0, 0.0f);
            ImGui::ResetMouseDragDelta(0);
            ImGui::SetScrollX(ImGui::GetScrollX() - drag_delta.x);
            ImGui::SetScrollY(ImGui::GetScrollY() - drag_delta.y);
          }
          if (!io.KeyCtrl) {
            uiUtils->imageClick(scale, io);
          }

          // End the child window
          ImGui::EndChild();
        }

        // images are less wide, on a usual 16x9 monitor, it is better to place
        // them besides each other
        if (aspectRatio <= 2.0)
          ImGui::SameLine();
        if (secondaryTexture != nullptr &&
            uiUtils->actTxs[0] != UIUtils::ActiveTexture::NONE) {
          ImGui::Image((void *)secondaryTexture, ImVec2(w * scale, h * scale));
        }
      }
      ImGui::End();
    }

    // Rendering
    uiUtils->renderImGui(g_pd3dDeviceContext, g_mainRenderTargetView,
                         clear_color, g_pSwapChain);
  }

  // Cleanup
  uiUtils->shutdownImGui();

  CleanupDeviceD3D();
  ::DestroyWindow(hwnd);
  ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
  return 0;
}

std::vector<std::string> GUI::loadConfigs() {
  std::vector<std::string> configSubfolders;
  for (const auto &entry : std::filesystem::directory_iterator("configs")) {
    if (entry.is_directory()) {
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
    system("pause");
  }
}
// hardcoded init of some game configs
void GUI::initGameConfigs() {
  gameConfigs.push_back({"Hearts of Iron IV", "hoi4"});
  gameConfigs.push_back({"Victoria 3", "vic3"});
  gameConfigs.push_back({"Europa Universalis IV", "eu4"});
  activeGameConfig = gameConfigs[0];
}

bool GUI::isRelevantModuleActive(const std::string &shortName) {
  return activeGameConfig.gameShortName == shortName;
}

// generic configure tab, containing a tab for fwg and rpdx configs
int GUI::showConfigure(Fwg::Cfg &cfg,
                       std::shared_ptr<Scenario::GenericModule> &activeModule) {
  if (ImGui::BeginTabItem("Configure")) {
    if (ImGui::BeginTabBar("Config tabs", ImGuiTabBarFlags_None)) {
      showRpdxConfigure(cfg, activeModule);
      showFwgConfigure(cfg, &curtexture);
      ImGui::EndTabBar();
    }
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showRpdxConfigure(
    Fwg::Cfg &cfg, std::shared_ptr<Scenario::GenericModule> &activeModule) {
  static int item_current = 1;
  static int selectedGame = 0;
  // remove the images, and set pretext for them to be auto
  // loaded after switching tabs again
  if (ImGui::BeginTabItem("RandomParadox Configuration")) {
    uiUtils->freeTexture(&curtexture);
    ImGui::PushItemWidth(200.0f);
    uiUtils->tabSwitchEvent();
    // find every subfolder of config folder
    if (!loadedConfigs) {
      loadedConfigs = true;
      configSubfolders = loadConfigs();
      activeConfig = configSubfolders[item_current];
    }

    std::vector<const char *> gameSelection;
    for (auto &gameConfig : gameConfigs) {
      gameSelection.push_back(gameConfig.gameName.c_str());
    }
    if (ImGui::ListBox("Game Selection", &selectedGame, gameSelection.data(),
                       gameSelection.size())) {
      if (gameConfigs[selectedGame].gameName == "Hearts of Iron IV") {
        activeModule = std::make_shared<Scenario::Hoi4::Hoi4Module>(
            Scenario::Hoi4::Hoi4Module(rpdConf, configSubFolder, username,
                                       false));
      } else if (gameConfigs[selectedGame].gameName ==
                 "Europa Universalis IV") {
        activeModule = std::make_shared<Scenario::Eu4::Module>(
            Scenario::Eu4::Module(rpdConf, configSubFolder, username));
      } else if (gameConfigs[selectedGame].gameName == "Victoria 3") {
        activeModule = std::make_shared<Scenario::Vic3::Module>(
            Scenario::Vic3::Module(rpdConf, configSubFolder, username));
      }
      activeGameConfig = gameConfigs[selectedGame];
      validatedPaths = false;
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
    }

    ImGui::PopItemWidth();
    if (ImGui::Button("Reload config")) {
      cfg.readConfig(activeConfig);
      loadGameConfig(cfg);
    }
    ImGui::PushItemWidth(600.0f);
    ImGui::InputText("Mod Name", &activeModule->pathcfg.modName);
    if (ImGui::Button("Try to find game")) {

      activeModule->findGame(activeModule->pathcfg.gamePath,
                             activeGameConfig.gameName);
    }
    if (ImGui::Button("Try to find mod folder")) {
      activeModule->autoLocateGameModFolder(activeGameConfig.gameName);
    }
    ImGui::InputText("Game Path", &activeModule->pathcfg.gamePath);
    if (ImGui::Button("Validate all paths")) {
      validatedPaths = activeModule->findGame(activeModule->pathcfg.gamePath,
                                              activeGameConfig.gameName);
      if (validatedPaths)
        validatedPaths =
            activeModule->validateGameModFolder(activeGameConfig.gameName);
      if (validatedPaths)
        validatedPaths =
            activeModule->validateModFolder(activeGameConfig.gameName);
    }
    ImGui::InputText("Mod Path", &activeModule->pathcfg.gameModPath);
    ImGui::InputText("Mods Directory",
                     &activeModule->pathcfg.gameModsDirectory);

    ImGui::PopItemWidth();
    ImGui::EndTabItem();
  }
  // force path for cutting to gamePath + maps, but only if no other path
  // defined
  if (cfg.cut) {
    cfg.loadMapsPath = activeModule->pathcfg.gamePath + "map//";
    cfg.heightmapIn = cfg.loadMapsPath + "heightmap.bmp";
  }
  if (cfg.loadClimate) {
    cfg.climateMappingPath = "resources//" + activeGameConfig.gameShortName +
                             "//climateMapping.txt ";
  }
  return 0;
}

void GUI::showModLoader(
    Fwg::Cfg &cfg, std::shared_ptr<Scenario::GenericModule> &genericModule) {
  if (ImGui::BeginTabItem("Modloader")) {
    if (triggeredDrag) {
      auto hoi4Module =
          std::reinterpret_pointer_cast<Scenario::Hoi4::Hoi4Module,
                                        Scenario::GenericModule>(genericModule);
      // hoi4Module->modEdit(draggedFile);
      triggeredDrag = false;
      uiUtils->resetTexture();
    }
    ImGui::EndTabItem();
  }
}

bool GUI::scenarioGenReady() {
  return configuredScenarioGen && !redoRegions && !redoProvinces;
}

int GUI::showScenarioTab(
    Fwg::Cfg &cfg, std::shared_ptr<Scenario::GenericModule> activeModule) {
  int retCode = 0;
  if (ImGui::BeginTabItem("Scenario")) {
    uiUtils->freeTexture(&curtexture);
    uiUtils->tabSwitchEvent();
    if (activeModule->generator->heightMap.initialised() &&
        activeModule->generator->climateMap.initialised() &&
        activeModule->generator->provinceMap.initialised() &&
        activeModule->generator->regionMap.initialised() &&
        activeModule->generator->worldMap.initialised()) {
      // auto initialize
      if (ImGui::Button("Init") ||
          !activeModule->generator->gameProvinces.size()) {
        if (!activeModule->createPaths()) {
          Fwg::Utils::Logging::logLine("ERROR: Couldn't create paths");
          retCode = -1;
        }
        activeModule->initNameData("resources//names",
                                   activeModule->pathcfg.gamePath);
        // start with the generic stuff in the Scenario Generator
        activeModule->generator->mapProvinces();
        activeModule->generator->mapRegions();
        activeModule->generator->mapContinents();
        configuredScenarioGen = true;
      }
      ImGui::PushItemWidth(200.0f);
      ImGui::InputDouble("WorldPopulationFactor",
                         &activeModule->generator->worldPopulationFactor, 0.1);
      ImGui::InputDouble("industryFactor",
                         &activeModule->generator->industryFactor, 0.1);
      if (isRelevantModuleActive("hoi4")) {
        auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
        showHoi4Configure(cfg, hoi4Gen);
      } else if (isRelevantModuleActive("vic3")) {
        auto vic3Gen = getGeneratorPointer<Vic3Gen>();
      } else if (isRelevantModuleActive("eu4")) {
        auto eu4Gen = getGeneratorPointer<Eu4Gen>();
      }
      ImGui::PopItemWidth();
    } else {
      ImGui::Text("Generate required maps in the other tabs first");
    }

    // switchTexture(fwg.provinceMap, texture, ActiveTexture::PROVINCES);
    ImGui::EndTabItem();
  }
  return retCode;
}

int GUI::showCountryTab(Fwg::Cfg &cfg, ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Countries")) {
    auto &generator = activeModule->generator;
    static int selectedStateIndex = 0;
    static std::string drawCountryTag;
    uiUtils->tabSwitchEvent(true);
    // pre-generate simply image even if no countries present
    if (!generator->countryMap.size()) {
      generator->dumpDebugCountrymap(cfg.mapsPath + "countries.png",
                                     generator->countryMap);
    }
    uiUtils->activeImage = &generator->countryMap;
    ImGui::Text(
        "Use auto generated country map or drop it in. You may also first "
        "generate a country map, then edit it in the Maps folder, and then "
        "drop it in again.");
    ImGui::Text("You can also drag in a list of countries (in a .txt file) "
                "with the following format: #r;g;b;tag;name;adjective. See "
                "inputs//countryMappings.txt as an example. If no file is "
                "dragged in, this example file is used.");
    ImGui::PushItemWidth(300.0f);
    ImGui::InputInt("Number of countries", &generator->numCountries);
    ImGui::InputText("Path to country list: ", &generator->countryMappingPath);
    if (ImGui::Checkbox("Draw-borders", &drawBorders)) {
      if (!drawBorders) {
        drawCountryTag = "";
      }
    }
    if (ImGui::Button("Generate countries")) {
      if (isRelevantModuleActive("hoi4")) {
        auto hoi4Gen = getGeneratorPointer<Hoi4Gen>();
        generator->generateCountries<Scenario::Hoi4::Hoi4Country>();
      } else if (isRelevantModuleActive("vic3")) {
        generator->generateCountries<Scenario::Vic3::Country>();
      } else if (isRelevantModuleActive("eu4")) {
        generator->generateCountries<Scenario::Country>();
      }
      // transfer generic states to hoi4states
      generator->initializeStates();
      // build hoi4 countries out of basic countries
      generator->initializeCountries();
      generator->evaluateNeighbours();
      generator->generateWorldCivilizations();
      generator->dumpDebugCountrymap(cfg.mapsPath + "countries.png",
                                     generator->countryMap);
      uiUtils->resetTexture();
    }
    ImGui::SameLine();
    auto str =
        "Generated countries: " + std::to_string(generator->countries.size());
    ImGui::Text(str.c_str());
    // drag event
    if (triggeredDrag) {
      if (draggedFile.find(".txt") != std::string::npos) {
        generator->countryMappingPath = draggedFile;
      } else {
        generator->loadCountries(draggedFile, generator->countryMappingPath);
        // transfer generic states to hoi4states
        generator->initializeStates();
        // build hoi4 countries out of basic countries
        generator->initializeCountries();
        generator->evaluateNeighbours();
        generator->generateWorldCivilizations();
        uiUtils->resetTexture();
      }
      triggeredDrag = false;
    }

    auto &clickEvents = uiUtils->clickEvents;
    if (clickEvents.size()) {
      auto pix = clickEvents.front();
      clickEvents.pop();
      const auto &colour = generator->provinceMap[pix.pixel];
      if (generator->areas.provinceColourMap.find(colour)) {
        const auto &prov = generator->areas.provinceColourMap[colour];
        auto &state = generator->gameRegions[prov->regionID];
        selectedStateIndex = state->ID;
      }
    }
    ImGui::Columns(3, "Edit"); // Start columns
    if (generator->gameRegions.size()) {
      auto &modifiableState = generator->gameRegions[selectedStateIndex];
      Elements::borderChild("StateEdit", [&]() {
        ImGui::InputText("State name", &modifiableState->name);
        ImGui::InputText("State owner", &modifiableState->owner);
        ImGui::InputInt("Population", &modifiableState->totalPopulation);
      });
      ImGui::NextColumn();
      if (isRelevantModuleActive("hoi4")) {
        const auto &hoi4Region =
            std::reinterpret_pointer_cast<Scenario::Hoi4::Region,
                                          Scenario::Region>(modifiableState);
        Elements::borderChild("StateEdit2", [&]() {
          ImGui::InputInt("Arms Industry", &hoi4Region->armsFactories);
          ImGui::InputInt("Civilian Industry", &hoi4Region->civilianFactories);
          ImGui::InputInt("Naval Industry", &hoi4Region->dockyards);
          ImGui::InputInt("State Category", &hoi4Region->stateCategory);
        });
      }
      ImGui::NextColumn();
      if ((modifiableState->owner.size() &&
               generator->countries.find(modifiableState->owner) !=
                   generator->countries.end() ||
           (drawCountryTag.size()) &&
               generator->countries.find(drawCountryTag) !=
                   generator->countries.end())) {
        std::shared_ptr selectedCountry = generator->countries.at(
            modifiableState->owner.size() ? modifiableState->owner
                                          : drawCountryTag);
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
            std::string &oldTag = selectedCountry->tag;
            generator->countries.erase(oldTag);
            selectedCountry->tag = bufferChangedTag;
            // add country under different tag
            generator->countries.insert(
                {selectedCountry->tag, selectedCountry});
            for (auto &region : selectedCountry->ownedRegions) {
              region->owner = selectedCountry->tag;
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
            // generator->countryMap =
            generator->dumpDebugCountrymap(cfg.mapsPath + "countries.png",
                                           generator->countryMap);
            uiUtils->resetTexture();
          }
        });

        if (drawBorders && drawCountryTag.size()) {
          if (generator->countries.find(drawCountryTag) !=
              generator->countries.end()) {
            selectedCountry = generator->countries.at(drawCountryTag);
          }
          if (!modifiableState->sea &&
              modifiableState->owner != selectedCountry->tag) {
            // modifiableState->owner = selectedCountry->tag;
            selectedCountry->addRegion(modifiableState);
            generator->dumpDebugCountrymap(cfg.mapsPath + "countries.png",
                                           generator->countryMap,
                                           modifiableState->ID);
            uiUtils->resetTexture();
          }
        }
      }
    }
    ImGui::Columns(1); // End columns
    uiUtils->switchTexture(activeModule->generator->countryMap, texture, 0,
                           UIUtils::ActiveTexture::EXTENDEABLE1, g_pd3dDevice,
                           w, h);
    ImGui::EndTabItem();
    ImGui::PopItemWidth();
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

int GUI::showModuleGeneric(
    Fwg::Cfg &cfg, std::shared_ptr<Scenario::GenericModule> genericModule) {
  if (!validatedPaths)
    ImGui::BeginDisabled();
  if (genericModule->generator->heightMap.initialised() &&
      genericModule->generator->climateMap.initialised() &&
      genericModule->generator->provinceMap.initialised() &&
      genericModule->generator->regionMap.initialised()) {
    if (ImGui::Button(
            std::string("Generate " + activeGameConfig.gameName + " mod")
                .c_str())) {
      genericModule->generate();
      configuredScenarioGen = true;
    }
  }
  ImGui::SameLine();
  if (ImGui::Button(std::string("Generate world + " +
                                activeGameConfig.gameName + " mod in one go")
                        .c_str())) {
    genericModule->generator->generateWorld();
    genericModule->generate();
    configuredScenarioGen = true;
  }
  if (!validatedPaths)
    ImGui::EndDisabled();
  return 0;
}

int GUI::showStrategicRegionTab(Fwg::Cfg &cfg,
                                std::shared_ptr<Hoi4Gen> generator) {
  if (ImGui::BeginTabItem("Strategic Regions")) {
    static int selectedStratRegionIndex = 0;
    // tab switch setting draw events as accepted
    uiUtils->tabSwitchEvent(true);
    if (!generator->stratRegionMap.size()) {
      activeModule->generator->stratRegionMap =
          generator->visualiseStrategicRegions();
    }
    uiUtils->activeImage = &generator->stratRegionMap;
    ImGui::SeparatorText(
        "This generates strategic regions, they cannot be loaded");
    if (generator->hoi4States.size()) {
      if (ImGui::Button("Generate strategic regions")) {
        // non-country stuff
        generator->generateStrategicRegions();
        generator->generateWeather();
        activeModule->generator->stratRegionMap =
            generator->visualiseStrategicRegions();
      }
      ImGui::Checkbox("Draw strategic regions", &drawBorders);
    } else {
      // transfer generic states to hoi4states
      generator->initializeStates();
      // build hoi4 countries out of basic countries
      generator->initializeCountries();
      generator->evaluateNeighbours();
      generator->generateWorldCivilizations();
    }
    // drag event is ignored here
    if (triggeredDrag) {
      triggeredDrag = false;
    }
    auto &clickEvents = uiUtils->clickEvents;
    if (clickEvents.size()) {
      auto pix = clickEvents.front();
      clickEvents.pop();
      const auto &colour = generator->provinceMap[pix.pixel];
      if (generator->areas.provinceColourMap.find(colour)) {
        const auto &prov = generator->areas.provinceColourMap[colour];
        auto &state = generator->gameRegions[prov->regionID];
        auto &stratRegion = generator->strategicRegions[state->superRegionID];
        if (drawBorders) {
          auto &rootRegion =
              generator->strategicRegions[selectedStratRegionIndex];
          stratRegion.removeRegion(state);
          rootRegion.addRegion(state);
        } else {
          selectedStratRegionIndex = stratRegion.ID;
        }
      }
      activeModule->generator->stratRegionMap =
          generator->visualiseStrategicRegions();
      uiUtils->resetTexture();
    }
    uiUtils->switchTexture(activeModule->generator->stratRegionMap, &curtexture,
                           0, UIUtils::ActiveTexture::EXTENDEABLE2,
                           g_pd3dDevice, w, h);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showHoi4Finalise(
    Fwg::Cfg &cfg, std::shared_ptr<Scenario::Hoi4::Hoi4Module> hoi4Module) {
  if (ImGui::BeginTabItem("Finalise")) {
    uiUtils->freeTexture(&curtexture);
    uiUtils->tabSwitchEvent();
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    auto &generator = hoi4Module->hoi4Gen;
    if (generator->strategicRegions.size()) {
      if (ImGui::Button("Export mod")) {
        // now generate hoi4 specific stuff
        generator->generateCountrySpecifics();
        generator->generateStateSpecifics();
        generator->generateStateResources();
        // should work with countries = 0
        generator->evaluateCountries();
        generator->generateLogistics();
        Scenario::Hoi4::NationalFocus::buildMaps();
        generator->generateFocusTrees();
        generator->generateCountryUnits();
        hoi4Module->writeImages();
        hoi4Module->writeTextFiles();
        generator->printStatistics();
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
//
