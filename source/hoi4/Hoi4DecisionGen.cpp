#include "hoi4/Hoi4DecisionGen.h"

namespace Rpx::Hoi4::DecisionGen {

void generateResourceDecisions(
    Rpx::Hoi4::DecisionData &decisionData, const int numDecisions,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions) {
  const auto &config = Fwg::Cfg::Values();
  std::vector<std::string> decisions;
  const std::vector<std::string> resources = {
      "coal", "oil", "steel", "aluminium", "rubber", "chromium", "tungsten"};
  // to ensure we don't generate the same resource decision multiple times for
  // the same state
  std::map<int, std::set<std::string>> stateResourceDecisions;
  const auto resourceTemplate = Fwg::Parsing::readFile(
      config.resourcePath + "/hoi4/common/decisions/resourceTemplate.txt");

  for (const auto &resource : resources) {
    for (int i = 0; i < numDecisions / 7; ++i) {
      std::string decision = resourceTemplate;

      const auto region = Fwg::Utils::Random::selectRandom(ardaRegions);
      // already have a decision for this resource in this state, skip
      if (stateResourceDecisions.contains(region->ID) &&
          stateResourceDecisions.at(region->ID).count(resource))
        continue;

      auto resourceAmount = RandNum::getRandom(2, 16);
      auto ppCost = resourceAmount * 4 + RandNum::getRandom(-4, 4);
      auto requiredExcavationLevel = RandNum::getRandom(1, 5);
      auto factoryCost = RandNum::getRandom(2, 4);

      Fwg::Parsing::replaceOccurences(decision, "templateStateID",
                                      std::to_string(region->ID + 1));
      Fwg::Parsing::replaceOccurences(decision, "templateResourceType",
                                      resource);
      Fwg::Parsing::replaceOccurences(decision, "templateResourceAmount",
                                      std::to_string(resourceAmount));
      Fwg::Parsing::replaceOccurences(decision, "templatePPCost",
                                      std::to_string(ppCost));
      Fwg::Parsing::replaceOccurences(decision, "templateExcavationLevel",
                                      std::to_string(requiredExcavationLevel));
      Fwg::Parsing::replaceOccurences(decision, "templateFactoryCost",
                                      std::to_string(factoryCost));

      stateResourceDecisions[region->ID].insert(resource);
      decisionData.decisionNames["develop_" + std::to_string(region->ID + 1) +
                                 "_" + resource + "_deposits"] =
          "Develop " + resource + " in " + region->name;
      decisions.push_back(decision);
    }
  }
  decisionData.resourceDecisions = decisions;
}

void generateDecisions(
    Rpx::Hoi4::DecisionData &decisionData,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions) {
  decisionData.decisionNames.clear();
  decisionData.resourceDecisions.clear();
  generateResourceDecisions(decisionData, 300, ardaRegions);
}

} // namespace Rpx::Hoi4::DecisionGen