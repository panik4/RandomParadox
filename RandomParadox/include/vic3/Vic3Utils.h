#pragma once
#include <vector>
namespace Scenario::Vic3 {
enum class ResourceType : int {
  COAL,
  FISH,
  GOLDFIELDS,
  GOLDMINES,
  IRON,
  LEAD,
  LOGGING,
  OIL,
  RUBBER,
  SULFUR,
  WHALING,
  LIVESTOCK,
  BANANA,
  COFFEE,
  COTTON,
  DYE,
  MAIZE,
  MILLET,
  OPIUM,
  RICE,
  RYE,
  SILK,
  SUGAR,
  TEA,
  TOBACCO,
  VINYARDS,
  WHEAT
};

struct Technology {};

struct TechnologyLevel {
  std::vector<Technology> minimumTech;
};
} // namespace Scenario::Vic3