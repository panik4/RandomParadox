#pragma once
#include "FastWorldGenerator.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#include <string>
#include <tchar.h>
#include <vector>

class GUI {
  void freeTexture(ID3D11ShaderResourceView **texture);
  // texture utils
  bool getResourceView(Fwg::Gfx::Bitmap &image,
                       ID3D11ShaderResourceView **out_srv, int *out_width,
                       int *out_height);
  enum class ActiveTexture {
    NONE,
    HEIGHTMAP,
    TERRAIN,
    CONTINENT,
    NORMALMAP,
    HUMIDITY,
    RIVER,
    CLIMATE,
    PROVINCES,
    REGIONS,
    COUNTRIES,
    STRATREGIONS
  };
  ActiveTexture actTx = ActiveTexture::NONE;
  void resetTexture();
  bool tabSwitchEvent();
  void switchTexture(Fwg::Gfx::Bitmap &image,
                     ID3D11ShaderResourceView **texture, ActiveTexture state);

  bool update = false;
  std::string draggedFile = "";
  bool triggeredDrag = false;
  float frequency = 0.0;
  int w;
  int h;
  static int seed;
  bool loadedConfigs = false;
  std::vector<std::string> configSubfolders;
  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();
  int showGeneric(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                  ID3D11ShaderResourceView **texture);
  std::vector<std::string> loadConfigs();
  int showConfigure(Fwg::Cfg &cfg, ID3D11ShaderResourceView **texture);
  int showHeightmapTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                       ID3D11ShaderResourceView **texture);
  int showTerrainTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                     ID3D11ShaderResourceView **texture);
  int showNormalMapTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                       ID3D11ShaderResourceView **texture);
  int showContinentTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                       ID3D11ShaderResourceView **texture);
  int showHumidityTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                      ID3D11ShaderResourceView **texture);
  int showRiverTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                   ID3D11ShaderResourceView **texture);
  int showClimateTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                     ID3D11ShaderResourceView **texture);
  int showProvincesTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                     ID3D11ShaderResourceView **texture);
  int showRegionTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                     ID3D11ShaderResourceView **texture);
  int showCountryTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                     ID3D11ShaderResourceView **texture);
  int showStrategicRegionTab(Fwg::Cfg &cfg, Fwg::FastWorldGenerator &fwg,
                     ID3D11ShaderResourceView **texture);


public:
  int shiny(Fwg::FastWorldGenerator &fwg);
};
