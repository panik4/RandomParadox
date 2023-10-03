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

int GUI::shiny(Scenario::Hoi4::Hoi4Module &hoi4Module) {
  this->pathconfig = hoi4Module.pathcfg;
  // this->fwg = generator;
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
                    L"ImGui Example",
                    nullptr};
  ::RegisterClassExW(&wc);
  HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example",
                              WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr,
                              nullptr, wc.hInstance, nullptr);
  MONITORINFO monitor_info;
  monitor_info.cbSize = sizeof(monitor_info);
  GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST),
                 &monitor_info);
  // gfx::Rect window_rect(monitor_info.rcMonitor);
  SetWindowPos(hwnd, NULL, 0, 0,
               monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
               monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
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
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  auto &cfg = Fwg::Cfg::Values();
  // Main loop
  bool done = false;
  //--- prior to main loop:
  DragAcceptFiles(hwnd, TRUE);
  curtexture = nullptr;

  // Fwg::generatorUI generatorUI;
  //  initial loading
  if (cfg.loadHeight) {
    hoi4Module.hoi4Gen.genHeight(cfg);
  }
  frequency = cfg.overallFrequencyModifier;
  // auto log = std::shared_ptr<std::stringstream>(new std::stringstream());
  log = std::make_shared<std::stringstream>();
  *log << Fwg::Utils::Logging::Logger::logInstance.getFullLog();
  Fwg::Utils::Logging::Logger::logInstance.attachStream(log);
  while (!done) {
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
            Fwg::Utils::Logging::logLine(filename);
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
      showGeneric(cfg, hoi4Module.hoi4Gen, &curtexture);
      ImGui::SameLine();
      showHoiGeneric(cfg, hoi4Module);
      ImGui::SeparatorText(
          "Different Steps of the generation, usually go from left to right");
      if (ImGui::BeginTabBar("Steps", ImGuiTabBarFlags_None)) {
        showConfigure(cfg, hoi4Module);
        showHeightmapTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showTerrainTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showNormalMapTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showContinentTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showClimateOverview(cfg, hoi4Module.hoi4Gen, &curtexture);
        showDensityTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showAreasTab(cfg, hoi4Module.hoi4Gen);
        showTreeTab(cfg, hoi4Module.hoi4Gen);
        showScenarioTab(cfg, hoi4Module);
        if (!configuredScenarioGen) {
          ImGui::BeginDisabled();
        }
        showCountryTab(cfg, hoi4Module, &curtexture);
        showStateTab(cfg, hoi4Module);
        showStrategicRegionTab(cfg, hoi4Module.hoi4Gen);
        showFinaliseTabConfigure(cfg, hoi4Module);
        if (!configuredScenarioGen) {
          ImGui::EndDisabled();
        }

        ImGui::EndTabBar();
      }
      float modif = 1.0 - (secondaryTexture != nullptr) * 0.5;
      auto scale =
          std::min<float>((ImGui::GetContentRegionAvail().y) / h,
                          (ImGui::GetContentRegionAvail().x) * modif / w);
      if (curtexture != nullptr) {
        ImGui::Image((void *)curtexture, ImVec2(w * scale, h * scale));
      }
      ImGui::SameLine();
      if (secondaryTexture != nullptr) {
        ImGui::Image((void *)secondaryTexture, ImVec2(w * scale, h * scale));
      }
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = {
        clear_color.x * clear_color.w, clear_color.y * clear_color.w,
        clear_color.z * clear_color.w, clear_color.w};
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView,
                                            nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                               clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
    // g_pSwapChain->Present(0, 0); // Present without vsync
  }

  // Cleanup
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDeviceD3D();
  ::DestroyWindow(hwnd);
  ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
  system("pause");
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
    Fwg::Parsing::replaceInStringStream(buffer, "\\", "//");

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

int GUI::showHoiGeneric(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module) {
  auto &hoi4Gen = hoi4Module.hoi4Gen;
  if (hoi4Gen.heightMap.initialized && hoi4Gen.climateMap.initialized &&
      hoi4Gen.provinceMap.initialized && hoi4Gen.regionMap.initialized) {
    if (ImGui::Button("Generate Hoi4Mod with default settings")) {
      hoi4Module.genHoi();
      configuredScenarioGen = true;
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Generate world + Hoi4 mod in one go")) {
    hoi4Gen.generateWorld();
    hoi4Module.genHoi();
    configuredScenarioGen = true;
  }
  return 0;
}
// generic configure tab, containing a tab for fwg and rpdx configs
int GUI::showConfigure(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module) {
  if (ImGui::BeginTabItem("Configure")) {
    if (ImGui::BeginTabBar("Config tabs", ImGuiTabBarFlags_None)) {
      showRpdxConfigure(cfg, hoi4Module);
      showFwgConfigure(cfg, &curtexture);
      ImGui::EndTabBar();
    }
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showRpdxConfigure(Fwg::Cfg &cfg,
                           Scenario::Hoi4::Hoi4Module &hoi4Module) {
  static int item_current = 1;
  // remove the images, and set pretext for them to be auto
  // loaded after switching tabs again
  if (ImGui::BeginTabItem("RandomParadox Configuration")) {
    freeTexture(&curtexture);
    tabSwitchEvent();
    // find every subfolder of config folder
    if (!loadedConfigs) {
      loadedConfigs = true;
      configSubfolders = loadConfigs();
      activeConfig = configSubfolders[item_current];
    }

    std::vector<const char *> items;

    ImGui::PushItemWidth(200.0f);
    for (auto &item : configSubfolders)
      items.push_back(item.c_str());
    ImGui::SeparatorText(
        "Click an entry in the list to choose a config preset");
    if (ImGui::ListBox("Config Presets", &item_current, items.data(),
                       items.size())) {
      activeConfig = items[item_current];
      Fwg::Utils::Logging::logLine("Switched to ", activeConfig,
                                   "\\FastWorldGenerator.json");
      cfg.readConfig(activeConfig);
      loadGameConfig(cfg);
    }

    ImGui::PopItemWidth();
    if (ImGui::Button("Reload config")) {
      cfg.readConfig(activeConfig);
      loadGameConfig(cfg);
    }
    ImGui::PushItemWidth(600.0f);
    ImGui::InputText("Mod Name", &hoi4Module.pathcfg.modName);
    if (ImGui::Button("Try to find game")) {
      hoi4Module.findGame(hoi4Module.pathcfg.gamePath, "Hearts of Iron IV");
    }
    ImGui::InputText("Game Path", &hoi4Module.pathcfg.gamePath);
    if (ImGui::Button("Validate mods directories")) {
      hoi4Module.findModFolders("Hearts of Iron IV");
    }
    ImGui::InputText("Mod Path", &hoi4Module.pathcfg.gameModPath);
    ImGui::InputText("Mods Directory", &hoi4Module.pathcfg.gameModsDirectory);

    ImGui::PopItemWidth();
    ImGui::EndTabItem();
  }
  // force path for cutting to gamePath + maps, but only if no other path
  // defined
  if (cfg.cut) {
    cfg.loadMapsPath = hoi4Module.pathcfg.gamePath + "map//";
    cfg.heightmapIn = cfg.loadMapsPath + "heightmap.bmp";
  }
  if (cfg.loadClimate) {
    cfg.climateMappingPath = "resources//hoi4//climateMapping.txt";
  }
  return 0;
}

int GUI::showHoi4Configure(Fwg::Cfg &cfg,
                           Scenario::Hoi4::Hoi4Module &hoi4Module) {

  ImGui::InputDouble("resourceFactor", &hoi4Module.hoi4Gen.resourceFactor, 0.1);
  ImGui::InputDouble("aluminiumFactor",
                     &hoi4Module.hoi4Gen.resources["aluminium"][2], 0.1);
  ImGui::InputDouble("chromiumFactor",
                     &hoi4Module.hoi4Gen.resources["chromium"][2], 0.1);
  ImGui::InputDouble("oilFactor", &hoi4Module.hoi4Gen.resources["oil"][2], 0.1);
  ImGui::InputDouble("rubberFactor", &hoi4Module.hoi4Gen.resources["rubber"][2],
                     0.1);
  ImGui::InputDouble("steelFactor", &hoi4Module.hoi4Gen.resources["steel"][2],
                     0.1);
  ImGui::InputDouble("tungstenFactor",
                     &hoi4Module.hoi4Gen.resources["tungsten"][2], 0.1);
  ImGui::InputDouble("baseLightRainChance",
                     &hoi4Module.hoi4Gen.weatherChances["baseLightRainChance"],
                     0.1);
  ImGui::InputDouble("baseHeavyRainChance",
                     &hoi4Module.hoi4Gen.weatherChances["baseHeavyRainChance"],
                     0.1);
  ImGui::InputDouble("baseMudChance",
                     &hoi4Module.hoi4Gen.weatherChances["baseMudChance"], 0.1);
  ImGui::InputDouble("baseBlizzardChance",
                     &hoi4Module.hoi4Gen.weatherChances["baseBlizzardChance"],
                     0.1);
  ImGui::InputDouble("baseSandstormChance",
                     &hoi4Module.hoi4Gen.weatherChances["baseSandstormChance"],
                     0.1);
  ImGui::InputDouble("baseSnowChance",
                     &hoi4Module.hoi4Gen.weatherChances["baseSnowChance"], 0.1);
  return 0;
}

int GUI::showScenarioTab(Fwg::Cfg &cfg,
                         Scenario::Hoi4::Hoi4Module &hoi4Module) {
  if (ImGui::BeginTabItem("Scenario")) {
    freeTexture(&curtexture);
    tabSwitchEvent();
    auto &generator = hoi4Module.hoi4Gen;
    if (generator.heightMap.initialized && generator.climateMap.initialized &&
        generator.provinceMap.initialized && generator.regionMap.initialized) {
      // auto initialize
      if (ImGui::Button("Init") || !generator.gameProvinces.size()) {
        if (!hoi4Module.createPaths())
          return -1;
        // start with the generic stuff in the Scenario Generator
        generator.mapProvinces();
        generator.mapRegions();
        generator.mapContinents();
        configuredScenarioGen = true;
      }
      ImGui::PushItemWidth(200.0f);
      ImGui::InputDouble("WorldPopulationFactor",
                         &hoi4Module.hoi4Gen.worldPopulationFactor, 0.1);
      ImGui::InputDouble("industryFactor", &hoi4Module.hoi4Gen.industryFactor,
                         0.1);
      showHoi4Configure(cfg, hoi4Module);
      ImGui::PopItemWidth();
    } else {
      ImGui::Text("Generate required maps in the other tabs first");
    }

    // switchTexture(fwg.provinceMap, texture, ActiveTexture::PROVINCES);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showCountryTab(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module,
                        ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Countries")) {
    auto &generator = hoi4Module.hoi4Gen;
    tabSwitchEvent();
    ImGui::Text(
        "Use auto generated country map or drop it in. You may also first "
        "generate a country map, then edit it in the Maps folder, and then "
        "drop it in again.");
    ImGui::Text("You can also drag in a list of countries (in a .txt file) "
                "with the following format: #r;g;b;tag;name;adjective. See "
                "inputs//countryMappings.txt as an example. If no file is "
                "dragged in, this example file is used.");
    ImGui::PushItemWidth(300.0f);
    ImGui::InputInt("Number of countries", &hoi4Module.numCountries);
    ImGui::InputText("Path to country list: ", &generator.countryMappingPath);
    if (ImGui::Button("Generate countries")) {
      generator.generateCountries(hoi4Module.numCountries, pathconfig.gamePath);
      // transfer generic states to hoi4states
      generator.initializeStates();
      // build hoi4 countries out of basic countries
      generator.initializeCountries();
      generator.evaluateNeighbours();
      generator.generateWorldCivilizations();
      generator.dumpDebugCountrymap(cfg.mapsPath + "countries.png");
      resetTexture();
    }
    ImGui::SameLine();
    auto str =
        "Generated countries: " + std::to_string(generator.countries.size());
    ImGui::Text(str.c_str());
    // drag event
    if (triggeredDrag) {
      if (draggedFile.find(".txt") != std::string::npos) {
        generator.countryMappingPath = draggedFile;
      } else {
        generator.loadCountries(draggedFile, generator.countryMappingPath);
        // transfer generic states to hoi4states
        generator.initializeStates();
        // build hoi4 countries out of basic countries
        generator.initializeCountries();
        generator.evaluateNeighbours();
        generator.generateWorldCivilizations();
        resetTexture();
      }
      triggeredDrag = false;
    }
    bool updateCountryMap = false;
    std::vector<const char *> items;
    std::vector<std::string> tags;
    static int selectedCountryIndex = 0;
    for (auto &country : generator.countries) {
      items.push_back(country.second.name.c_str());
      tags.push_back(country.first);
    }
    if (generator.countries.size()) {
      ImGui::ListBox("Select Country", &selectedCountryIndex, items.data(),
                     items.size(), 4);
      auto selectedCountry = generator.countries[tags[selectedCountryIndex]];
      std::string tempTag = selectedCountry.tag;
      static std::string bufferChangedTag = "";
      if (ImGui::InputText("Country tag", &tempTag)) {
        bufferChangedTag = tempTag;
      }
      if (ImGui::Button("update tag")) {
        std::string &oldTag = tags[selectedCountryIndex];
        generator.countries.erase(oldTag);
        selectedCountry.tag = bufferChangedTag;
        // add country under different tag
        generator.countries.insert({selectedCountry.tag, selectedCountry});
        // update old tag in list
        tags[selectedCountryIndex] = selectedCountry.tag;
      }
      auto &modifiableCountry = generator.countries[tags[selectedCountryIndex]];
      ImGui::InputText("Country name", &modifiableCountry.name);
      ImGui::InputText("Country adjective", &modifiableCountry.adjective);
      // ImGui::InputText("Country adjective", &selectedCountry.colour);
      ImVec4 color =
          ImVec4(((float)modifiableCountry.colour.getRed()) / 255.0f,
                 ((float)modifiableCountry.colour.getGreen()) / 255.0f,
                 ((float)modifiableCountry.colour.getBlue()) / 255.0f, 1.0f);

      if (ImGui::ColorEdit3("Country Colour", (float *)&color,
                            ImGuiColorEditFlags_NoInputs |
                                ImGuiColorEditFlags_NoLabel |
                                ImGuiColorEditFlags_HDR)) {
        modifiableCountry.colour =
            Fwg::Gfx::Colour(color.x * 255.0, color.y * 255.0, color.z * 255.0);
        if (ImGui::IsMouseClicked) {
          generator.countryMap =
              generator.dumpDebugCountrymap(cfg.mapsPath + "countries.png");
          resetTexture();
        }
      }
    }

    switchTexture(hoi4Module.hoi4Gen.countryMap, texture,
                  ActiveTexture::COUNTRIES);
    ImGui::EndTabItem();
    ImGui::PopItemWidth();
  }
  return 0;
}

int GUI::showStateTab(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module) {
  if (ImGui::BeginTabItem("States")) {
    auto &generator = hoi4Module.hoi4Gen;
    tabSwitchEvent();
    ImGui::Text(
        "Use auto generated country map or drop it in. You may also first "
        "generate a country map, then edit it in the Maps folder, and then "
        "drop it in again.");
    ImGui::Text("You can also drag in a list of states (in a .txt file) "
                "with the following format: #r;g;b;tag;name;population. See "
                "inputs//countryMappings.txt as an example. If no file is "
                "dragged in, this example file is used.");
    ImGui::PushItemWidth(300.0f);
    ImGui::InputText("Path to country list: ", &generator.regionMappingPath);
    ImGui::SameLine();
    auto str =
        "Amount of states: " + std::to_string(generator.hoi4States.size());
    ImGui::Text(str.c_str());
    // drag event
    if (triggeredDrag) {
      if (draggedFile.find(".txt") != std::string::npos) {
        generator.regionMappingPath = draggedFile;
        generator.applyRegionInput();
      }
      triggeredDrag = false;
    }
    std::vector<const char *> items;
    static int selectedStateIndex = 0;
    for (auto &state : generator.hoi4States) {
      items.push_back(state->name.c_str());
    }
    if (generator.countries.size()) {
      ImGui::ListBox("Select State", &selectedStateIndex, items.data(),
                     items.size(), 4);
      auto &modifiableState = generator.hoi4States[selectedStateIndex];
      ImGui::InputText("Country name", &modifiableState->name);
      ImGui::InputInt("Population", &modifiableState->population);
      ImGui::InputInt("Arms Industry", &modifiableState->armsFactories);
      ImGui::InputInt("Civilian Industry", &modifiableState->civilianFactories);
      ImGui::InputInt("Naval Industry", &modifiableState->dockyards);
      ImGui::InputInt("State Category", &modifiableState->stateCategory);
    }

    ImGui::EndTabItem();
    ImGui::PopItemWidth();
  }
  return 0;
}

int GUI::showStrategicRegionTab(Fwg::Cfg &cfg,
                                Scenario::Hoi4::Generator &generator) {
  if (ImGui::BeginTabItem("Strategic Regions")) {
    freeTexture(&curtexture);
    tabSwitchEvent();
    ImGui::SeparatorText(
        "This generates strategic regions, they cannot be loaded");
    if (generator.hoi4States.size()) {
      if (ImGui::Button("Generate strategic regions")) {
        // non-country stuff
        generator.generateStrategicRegions();
        generator.generateWeather();
      }
    } else {
      // transfer generic states to hoi4states
      generator.initializeStates();
      // build hoi4 countries out of basic countries
      generator.initializeCountries();
      generator.evaluateNeighbours();
      generator.generateWorldCivilizations();
    }
    // drag event is ignored here
    if (triggeredDrag) {
      triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showFinaliseTabConfigure(Fwg::Cfg &cfg,
                                  Scenario::Hoi4::Hoi4Module &hoi4Module) {
  if (ImGui::BeginTabItem("Finalise")) {
    freeTexture(&curtexture);
    tabSwitchEvent();
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    auto &hoi4Gen = hoi4Module.hoi4Gen;
    if (hoi4Gen.strategicRegions.size()) {
      if (ImGui::Button("Export mod")) {
        // now generate hoi4 specific stuff
        hoi4Gen.generateCountrySpecifics();
        hoi4Gen.generateStateSpecifics();
        hoi4Gen.generateStateResources();
        // should work with countries = 0
        hoi4Gen.evaluateCountries();
        hoi4Gen.generateLogistics();
        Scenario::Hoi4::NationalFocus::buildMaps();
        hoi4Gen.generateFocusTrees();
        hoi4Gen.generateCountryUnits();
        hoi4Module.writeImages();
        hoi4Module.writeTextFiles();
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
