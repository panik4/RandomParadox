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
            std::cout << filename << std::endl;
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
      ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

      ImGui::SetNextWindowPos({0, 0});
      ImGui::SetNextWindowSize({io.DisplaySize.x, io.DisplaySize.y});
      ImGui::Begin("RandomParadox");
      showGeneric(cfg, hoi4Module.hoi4Gen, &curtexture);
      if (ImGui::BeginTabBar("Steps", tab_bar_flags)) {
        showConfigure(cfg, &curtexture);
        showFwgConfigure(cfg, &curtexture);
        showHeightmapTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showTerrainTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showNormalMapTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showContinentTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showClimateOverview(cfg, hoi4Module.hoi4Gen, &curtexture);
        showDensityTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showProvincesTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showRegionTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showTreeTab(cfg, hoi4Module.hoi4Gen);
        showHoi4Configure(cfg, hoi4Module, &curtexture);
        if (!configuredScenarioGen) {
          ImGui::BeginDisabled();
        }
        showCountryTab(cfg, hoi4Module, &curtexture);
        showStrategicRegionTab(cfg, hoi4Module.hoi4Gen, &curtexture);
        showFinaliseTabConfigure(cfg, hoi4Module);
        if (!configuredScenarioGen) {
          ImGui::EndDisabled();
        }

        ImGui::EndTabBar();
      }
      auto scale = (ImGui::GetWindowHeight() * 0.5) / h;
      if (curtexture != nullptr)
        ImGui::Image((void *)curtexture, ImVec2(w * scale, h * scale));
      ImGui::SameLine();
      if (secondaryTexture != nullptr)
        ImGui::Image((void *)secondaryTexture, ImVec2(w * scale, h * scale));
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
      std::cout << entry << std::endl;
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

  // overwrites for fwg
  cfg.loadMapsPath = hoi4Conf.get<std::string>("fastworldgen.loadMapsPath");
  cfg.heightmapIn = cfg.loadMapsPath +
                    hoi4Conf.get<std::string>("fastworldgen.heightMapName");
}

int GUI::showConfigure(Fwg::Cfg &cfg, ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Configure")) {
    // remove the images, and set pretext for them to be auto loaded after
    // switching tabs again
    freeTexture(texture);
    tabSwitchEvent();
    // find every subfolder of config folder
    if (!loadedConfigs) {
      loadedConfigs = true;
      configSubfolders = loadConfigs();
      activeConfig = cfg.path;
    }
    if (ImGui::Button("Reload config")) {
      cfg.readConfig(activeConfig);
      loadGameConfig(cfg);
    }
    std::vector<const char *> items;
    for (auto &item : configSubfolders)
      items.push_back(item.c_str());
    static int item_current = 1;
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

    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showHoi4Configure(Fwg::Cfg &cfg,
                           Scenario::Hoi4::Hoi4Module &hoi4Module,
                           ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Hoi4 config")) {
    if (ImGui::Button("Init")) {
      auto &generator = hoi4Module.hoi4Gen;
      if (!hoi4Module.createPaths())
        return -1;
      // start with the generic stuff in the Scenario Generator
      generator.mapProvinces();
      generator.mapRegions();
      generator.mapContinents();
      configuredScenarioGen = true;
    }
    ImGui::EndTabItem();
  }

  return 0;
}

int GUI::showCountryTab(Fwg::Cfg &cfg, Scenario::Hoi4::Hoi4Module &hoi4Module,
                        ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Countries")) {
    auto &generator = hoi4Module.hoi4Gen;
    tabSwitchEvent();
    ImGui::InputInt("Number of countries", &hoi4Module.numCountries);
    ImGui::Text(
        "Use auto generated country map or drop it in. You may also first "
        "generate a country map, then edit it in the Maps folder, and then "
        "drop it in again");
    if (ImGui::Button("Generate countries")) {
      generator.generateCountries(hoi4Module.numCountries, pathconfig.gamePath);
      // transfer generic states to hoi4states
      generator.initializeStates();
      // build hoi4 countries out of basic countries
      generator.initializeCountries();
      generator.evaluateNeighbours();
      generator.generateWorldCivilizations();
      Fwg::Gfx::Bitmap countryMap =
          generator.dumpDebugCountrymap(cfg.mapsPath + "countries.png");
    }
    // drag event
    if (triggeredDrag) {
      // generator.count = Fwg::IO::Reader::readGenericImage(draggedFile, cfg);
      generator.loadCountries(draggedFile, generator.countryMappingPath);
      // transfer generic states to hoi4states
      generator.initializeStates();
      // build hoi4 countries out of basic countries
      generator.initializeCountries();
      generator.evaluateNeighbours();
      generator.generateWorldCivilizations();
      triggeredDrag = false;
      resetTexture();
    }
    // switchTexture(generator., texture, ActiveTexture::COUNTRIES);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showStrategicRegionTab(Fwg::Cfg &cfg,
                                Scenario::Hoi4::Generator &generator,
                                ID3D11ShaderResourceView **texture) {
  return 0;
}

int GUI::showFinaliseTabConfigure(Fwg::Cfg &cfg,
                                  Scenario::Hoi4::Hoi4Module &hoi4Module) {
  if (ImGui::BeginTabItem("Finalise")) {
    freeTexture(&curtexture);
    tabSwitchEvent();
    ImGui::Text("This will finish generating the mod and write it to the "
                "configured paths");
    if (ImGui::Button("Finish mod")) {
      auto &hoi4Gen = hoi4Module.hoi4Gen;
      // non-country stuff
      hoi4Gen.generateStrategicRegions();
      hoi4Gen.generateWeather();
      // now generate hoi4 specific stuff
      hoi4Gen.generateCountrySpecifics();
      hoi4Gen.generateStateSpecifics();
      hoi4Gen.generateStateResources();
      // should work with countries = 0
      hoi4Gen.evaluateCountries();
      Fwg::Gfx::Bitmap countryMap =
          hoi4Gen.dumpDebugCountrymap(cfg.mapsPath + "countries.png");
      hoi4Gen.generateLogistics(countryMap);
      Scenario::Hoi4::NationalFocus::buildMaps();
      hoi4Gen.generateFocusTrees();
      hoi4Gen.generateCountryUnits();
      hoi4Module.writeImages();
      hoi4Module.writeTextFiles();
    }
    // drag event is ignored here
    if (triggeredDrag) {
      triggeredDrag = false;
    }
    ImGui::EndTabItem();
  }

  return 0;
}
