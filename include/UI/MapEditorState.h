#pragma once

#include <string>

namespace Rpx::UI {

enum class MapEditorLayer {
  Countries,
  States,
  Provinces,
  Locations,
  Continents,
  StrategicRegions
};

struct MapEditorState {
  MapEditorLayer activeLayer = MapEditorLayer::Countries;
  int selectedStateID = -1;
	int selectedProvinceID = -1;
  int selectedLocationID = -1;
  int selectedContinentID = -1;
  std::string selectedCountryTag;
  std::string borderPaintCountryTag;
  bool drawBorders = false;
};

} // namespace Rpx::UI
