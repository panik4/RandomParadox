#include "UI/GUI.h"
// Data
static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
int GUI::seed = 0;

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

int GUI::shiny(Fwg::FastWorldGenerator &fwg) {
  // this->fwg = fwg;
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
  ID3D11ShaderResourceView *curtexture = nullptr;

  // initial loading
  if (cfg.loadHeight) {
    fwg.genHeight(cfg);
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
      showGeneric(cfg, fwg, &curtexture);
      if (ImGui::BeginTabBar("Steps", tab_bar_flags)) {
        showConfigure(cfg, &curtexture);
        showHeightmapTab(cfg, fwg, &curtexture);
        showTerrainTab(cfg, fwg, &curtexture);
        showNormalMapTab(cfg, fwg, &curtexture);
        showContinentTab(cfg, fwg, &curtexture);
        showHumidityTab(cfg, fwg, &curtexture);
        showRiverTab(cfg, fwg, &curtexture);
        showClimateTab(cfg, fwg, &curtexture);
        showProvincesTab(cfg, fwg, &curtexture);
        showRegionTab(cfg, fwg, &curtexture);
        showCountryTab(cfg, fwg, &curtexture);
        showStrategicRegionTab(cfg, fwg, &curtexture);
        ImGui::EndTabBar();
      }
      if (curtexture != nullptr)
        ImGui::Image((void *)curtexture, ImVec2(w, h));
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

int GUI::showGeneric(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                      ID3D11ShaderResourceView **texture) {
  if (ImGui::InputInt("Seed", &cfg.seed)) {
    cfg.randomSeed = false;
    cfg.reRandomize();
  }
  if (ImGui::Button("Get random seed")) {
    cfg.randomSeed = true;
    cfg.reRandomize();
  }
  if (ImGui::Button("Generate remaining")) {
    fwg.generateWorld();
  }

  ImGui::Text("size = %d x %d", w, h);
  return 0;
}

void GUI::freeTexture(ID3D11ShaderResourceView **texture) {
  if ((*texture) != nullptr) {
    (*texture)->Release();
    *texture = nullptr;
  }
}

void GUI::resetTexture() { actTx = ActiveTexture::NONE; }

bool GUI::tabSwitchEvent() {
  if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered()) {
    actTx = ActiveTexture::NONE;
    return true;
  }
  return false;
}

void GUI::switchTexture(Fwg::Gfx::Bitmap &image,
                         ID3D11ShaderResourceView **texture,
                         ActiveTexture state) {
  if (actTx != state) {
    freeTexture(texture);
    if (image.initialized) {
      getResourceView(image, texture, &w, &h);
      actTx = state;
    }
  }
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
    }
    std::vector<const char *> items;
    for (auto &item : configSubfolders)
      items.push_back(item.c_str());
    static int item_current = 1;
    ImGui::ListBox("Config Presets", &item_current, items.data(), items.size());

    ImGui::EndTabItem();
  }
  return 0;
}
int GUI::showHeightmapTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                           ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Heightmap")) {
    tabSwitchEvent();
    int tmpLayers = cfg.layerAmount;
    ImGui::InputInt("Layers", &tmpLayers);
    if (ImGui::SliderFloat("Overall Frequency", &frequency, 0.1f, 10.0f,
                           "ratio = %.1f")) {
    } else {
      if (!ImGui::IsMouseDown(0) &&
          frequency != (float)cfg.overallFrequencyModifier) {
        cfg.overallFrequencyModifier = frequency;
        update = true;
      }
    }
    tmpLayers = std::clamp(tmpLayers, 1, cfg.maxLayerAmount);
    cfg.layerAmount = tmpLayers;
    if (update ||
        ImGui::Button("Generate a heightmap or drop it in from the file "
                      "explorer")) {
      cfg.loadHeight = false;
      fwg.genHeight(cfg);
      resetTexture();
      update = false;
    }

    // drag event
    if (triggeredDrag) {
      fwg.heightMap = Fwg::IO::Reader::readHeightmapImage(
          draggedFile, cfg, fwg.originalHeightMap);
      triggeredDrag = false;
    }

    switchTexture(fwg.heightMap, texture, ActiveTexture::HEIGHTMAP);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showTerrainTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                         ID3D11ShaderResourceView **texture) {

  if (ImGui::BeginTabItem("Terrain")) {
    tabSwitchEvent();
    ImGui::InputInt("SeaLevel", &cfg.seaLevel);
    ImGui::InputDouble("Landpercentage", &cfg.landPercentage);
    if (ImGui::Button("Generate a simple overview of land area from heightmap "
                      "or drop it in")) {
      fwg.genTerrain(cfg);
      resetTexture();
    }

    // drag event
    if (triggeredDrag) {
      if (fwg.heightMap.initialized && ImGui::BeginPopup("my_select_popup")) {
        ImGui::SeparatorText(
            "You are trying to load a terrain map despite having a heightmap "
            "already loaded. If you want to generate from this basic shape, "
            "click yes, otherwise click no.");
        if (ImGui::Selectable(
                "Yes - this will overwrite the other heightmap")) {
          fwg.genHeightFromSimpleInput(cfg, draggedFile);
          fwg.genTerrain(cfg);
        }
        if (ImGui::Selectable(
                "No - you can simply press generate to generate "
                "basic terrain shape from the already loaded heightmap")) {
          fwg.genHeightFromSimpleInput(cfg, draggedFile);
          fwg.genTerrain(cfg);
        }
        ImGui::EndPopup();
      } else {
        fwg.genHeightFromSimpleInput(cfg, draggedFile);
        fwg.genTerrain(cfg);
      }
      triggeredDrag = false;
    }

    switchTexture(fwg.terrainMap, texture, ActiveTexture::TERRAIN);
    ImGui::EndTabItem();
  }
  return 0;
}
int GUI::showNormalMapTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                           ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Normalmap")) {
    tabSwitchEvent();
    ImGui::InputInt("Sobelfactor", &cfg.sobelFactor);
    if (ImGui::Button("Generate Normalmap")) {
      fwg.genSobelMap(cfg);
      resetTexture();
    }
    switchTexture(fwg.sobelMap, texture, ActiveTexture::NORMALMAP);
    ImGui::EndTabItem();
  }
  return 0;
}
int GUI::showContinentTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                           ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Continents")) {
    tabSwitchEvent();
    switchTexture(fwg.continentMap, texture, ActiveTexture::CONTINENT);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showHumidityTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                          ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Humidity")) {
    tabSwitchEvent();
    if (ImGui::Button("Generate Normalmap")) {
      fwg.genSobelMap(cfg);
      resetTexture();
    }
    switchTexture(fwg.humidityMap, texture, ActiveTexture::HUMIDITY);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showRiverTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                       ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Rivers")) {
    tabSwitchEvent();
    if (ImGui::Button("Generate River Map")) {
      fwg.genRivers(cfg);
      resetTexture();
    }
    switchTexture(fwg.riverMap, texture, ActiveTexture::RIVER);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showClimateTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                         ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Climate")) {
    tabSwitchEvent();
    if (ImGui::Button("Generate Climate Map")) {
      fwg.genClimate(cfg);
      resetTexture();
    }
    switchTexture(fwg.climateMap, texture, ActiveTexture::CLIMATE);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showProvincesTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                           ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Provinces")) {
    tabSwitchEvent();
    if (ImGui::Button("Generate Provinces Map")) {
      fwg.genProvinces(cfg);
      resetTexture();
    }
    switchTexture(fwg.provinceMap, texture, ActiveTexture::PROVINCES);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showRegionTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                        ID3D11ShaderResourceView **texture) {
  if (ImGui::BeginTabItem("Regions")) {
    tabSwitchEvent();
    if (ImGui::Button("Generate Region Map")) {
      fwg.genRegions(cfg);
      resetTexture();
    }
    switchTexture(fwg.regionMap, texture, ActiveTexture::REGIONS);
    ImGui::EndTabItem();
  }
  return 0;
}

int GUI::showCountryTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                         ID3D11ShaderResourceView **texture) {
  return 0;
}

int GUI::showStrategicRegionTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                               ID3D11ShaderResourceView **texture) {
  return 0;
}

// Helper functions

bool GUI::CreateDeviceD3D(HWND hWnd) {
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };
  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
      &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
  if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software
                                     // driver if hardware is not available.
    res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
  if (res != S_OK)
    return false;

  CreateRenderTarget();
  return true;
}

void GUI::CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
  }
  if (g_pd3dDeviceContext) {
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = nullptr;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }
}

void GUI::CreateRenderTarget() {
  ID3D11Texture2D *pBackBuffer;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                       &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void GUI::CleanupRenderTarget() {
  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
  }
}
// Simple helper function to load an image into a DX11 texture with common
// settings
bool GUI::getResourceView(Fwg::Gfx::Bitmap &image,
                           ID3D11ShaderResourceView **out_srv, int *out_width,
                           int *out_height) {
  // Load from disk into a raw RGBA buffer
  int image_width = image.bInfoHeader.biWidth;
  int image_height = image.bInfoHeader.biHeight;
  std::vector<unsigned char> image_data = image.get32bit();

  // Create texture
  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = image_width;
  desc.Height = image_height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;

  ID3D11Texture2D *pTexture = NULL;
  D3D11_SUBRESOURCE_DATA subResource;
  subResource.pSysMem = image_data.data();
  subResource.SysMemPitch = desc.Width * 4;
  subResource.SysMemSlicePitch = 0;
  g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

  // Create texture view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  ZeroMemory(&srvDesc, sizeof(srvDesc));
  srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = desc.MipLevels;
  srvDesc.Texture2D.MostDetailedMip = 0;
  g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
  pTexture->Release();

  *out_width = image_width;
  *out_height = image_height;
  return true;
}