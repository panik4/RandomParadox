#pragma once
#include "Culture.h"
#include "NameGenerator.h"
#include "entities/Colour.h"
#include "generic/GameRegion.h"
#include <string>
namespace Scenario {

class CultureGroup {
  std::string name;
  std::vector<std::shared_ptr<Culture>> cultures;
  Fwg::Gfx::Colour colour;
  std::shared_ptr<Scenario::LanguageGroup> languageGroup;
  std::shared_ptr<Region> center;
  std::vector<std::shared_ptr<Region>> regions;
  VisualType visualType;

public:
  // Constructor
  CultureGroup(const std::string &name, const Fwg::Gfx::Colour &colour)
      : name(name), colour(colour) {}

  // Method to add a culture
  void addCulture(const std::shared_ptr<Culture> &culture) {
    cultures.push_back(culture);
  }

  // Method to remove a culture
  void removeCulture(const std::shared_ptr<Culture> &culture) {
    culture->cultureGroup = nullptr;
    cultures.erase(std::remove(cultures.begin(), cultures.end(), culture),
                   cultures.end());
  }

  void addRegion(const std::shared_ptr<Region> &region) {
    regions.push_back(region);
  }

  void removeRegion(const std::shared_ptr<Region> &region) {
    regions.erase(std::remove(regions.begin(), regions.end(), region),
                  regions.end());
  }

  void setCenter(const std::shared_ptr<Region> &region) { center = region; }
  void setLanguageGroup(const std::shared_ptr<Scenario::LanguageGroup> &group) {
    languageGroup = group;
  }
  std::shared_ptr<Region> getCenter() { return center; }
  std::vector<std::shared_ptr<Region>> getRegions() { return regions; }
  std::vector<std::shared_ptr<Culture>> getCultures() { return cultures; }
  std::string getName() { return name; }
  Fwg::Gfx::Colour getColour() { return colour; }
  std::shared_ptr<Scenario::LanguageGroup> getLanguageGroup() {
    return languageGroup;
  }

  // determines the visual looks of the culture group according to their
  // geographic location
  void determineVisualType();

  VisualType getVisualType() { return visualType; }
};

} // namespace Scenario