#include "hoi4/Hoi4Army.h"
namespace Scenario::Hoi4 {

DivisionTemplate createUniformTemplate(int totalRegiments, int &columns,
                                       const CombatRegimentType &type) {
  DivisionTemplate divisionTemplate;
  divisionTemplate.type = DivisionType::Infantry;
  // if we have more than 8 regiments, we need at least three columns if we
  // havent this many alredy

  if (totalRegiments > 8) {
    columns = std::max<int>(columns, 3);
  }

  int regimentAmount = totalRegiments / columns;
  int leftOverRegiments = totalRegiments % columns;
  divisionTemplate.regiments.resize(columns);
  for (int i = 0; i < columns; i++) {
    for (int j = 0; j < regimentAmount; j++) {
      divisionTemplate.regiments[i].push_back(type);
    }
    // check if we have any left over regiments
    if (leftOverRegiments > 0) {
      divisionTemplate.regiments[i].push_back(type);
      leftOverRegiments--;
    }
  }
  return divisionTemplate;
}

DivisionTemplate createInfantryTemplate(int totalRegiments, int &columns) {
  DivisionTemplate divisionTemplate = createUniformTemplate(
      totalRegiments, columns, CombatRegimentType::Infantry);

  divisionTemplate.type = DivisionType::Infantry;
  return divisionTemplate;
}

DivisionTemplate createCavalryTemplate(int totalRegiments, int columns) {
  DivisionTemplate divisionTemplate = createUniformTemplate(
      totalRegiments, columns, CombatRegimentType::Cavalry);
  divisionTemplate.type = DivisionType::Cavalry;

  return divisionTemplate;
}

DivisionTemplate createMotorizedTemplate(int totalRegiments, int columns) {
  DivisionTemplate divisionTemplate = createUniformTemplate(
      totalRegiments, columns, CombatRegimentType::Motorized);
  divisionTemplate.type = DivisionType::Motorized;

  return divisionTemplate;
}

DivisionTemplate createArmorTemplate(int totalRegiments, int columns) {
  DivisionTemplate divisionTemplate = createUniformTemplate(
      totalRegiments, columns, CombatRegimentType::LightArmor);
  divisionTemplate.type = DivisionType::Armor;

  return divisionTemplate;
}

void addSupportRegiments(
    DivisionTemplate &divisionTemplate,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes,
    int maximumSupportTemplates) {
  auto tempSupportRegiments = availableSupportRegimentTypes;
  // we fill the supportRegiments vector with random support regiments, but they
  // must be unique in type we also check if we have the support regiment type
  for (int i = 0; i < maximumSupportTemplates; i++) {
    if (tempSupportRegiments.empty()) {
      break;
    }
    auto randomIndex =
        tempSupportRegiments.size() == 1
            ? 0
            : RandNum::getRandom(0, (int)tempSupportRegiments.size() - 1);
    divisionTemplate.supportRegiments.push_back(
        tempSupportRegiments[randomIndex]);
    tempSupportRegiments.erase(tempSupportRegiments.begin() + randomIndex);
  }
  std::cout << "Supp: " + divisionTemplate.supportRegiments.size() << std::endl;
}

// this
void addCombatSupportRegiments(
    DivisionTemplate &divisionTemplate,
    const std::vector<CombatRegimentType> &availableCombatRegimentTypes,
    int maximumSupportTemplates) {
  // we fill another column with combat support regiments randomly from the
  // availableCombatRegimentTypes, we add a new column first
  divisionTemplate.regiments.resize(divisionTemplate.regiments.size() + 1);
  for (int i = 0; i < maximumSupportTemplates; i++) {
    if (availableCombatRegimentTypes.empty()) {
      break;
    }
    auto randomIndex =
        RandNum::getRandom(0, (int)availableCombatRegimentTypes.size() - 1);
    divisionTemplate.regiments.back().push_back(
        {availableCombatRegimentTypes[randomIndex]});
  }
  std::cout << divisionTemplate.regiments.back().size() << std::endl;
}

// this adds line artillery regiments to the division
void addArtilleryRegiments(
    DivisionTemplate &divisionTemplate,
    const std::vector<CombatRegimentType> &availableRegimentTypes,
    int maximumArtilleryTemplates) {
  // we fill another new column with artilleryRegiments, but first check if we
  // have Artillery as a support regiment
  // first check if we have Artillery
  bool hasArtillery = false;
  for (auto type : availableRegimentTypes) {
    if (type == CombatRegimentType::Artillery) {
      hasArtillery = true;
      break;
    }
  }
  if (!hasArtillery) {
    return;
  }
  // we add a new column
  divisionTemplate.regiments.resize(divisionTemplate.regiments.size() + 1);
  for (int i = 0; i < maximumArtilleryTemplates; i++) {
    if (divisionTemplate.type == DivisionType::HeavyArtilleryInfantry) {
      divisionTemplate.regiments.back().push_back(
          CombatRegimentType::Artillery);
    } else if (divisionTemplate.type == DivisionType::HeavyArtilleryMotorized) {
      divisionTemplate.regiments.back().push_back(
          CombatRegimentType::MotorizedArtillery);
    }
  }
}

// calculate the cost of a division
double calculateDivisionCost(const DivisionTemplate &divisionTemplate) {
  double cost = 0.0;
  // we add the cost of the regiments
  for (const auto &column : divisionTemplate.regiments) {
    for (const auto &regiment : column) {
      cost += regimentCost.at(regiment);
    }
  }
  // we add the cost of the support regiments
  for (const auto &regiment : divisionTemplate.supportRegiments) {
    cost += supportRegimentCost.at(regiment);
  }
  return cost;
}

std::vector<DivisionTemplate> Scenario::Hoi4::createDivisionTemplates(
    const std::set<DivisionType> &desiredTypes,
    const std::vector<CombatRegimentType> &availableRegimentTypes,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes) {
  // first gather all the CombatSupportRegimentTypes
  std::vector<CombatRegimentType> combatSupportRegimentTypes;
  for (auto type : availableRegimentTypes) {
    if (type == CombatRegimentType::AntiAir ||
        type == CombatRegimentType::AntiTank ||
        type == CombatRegimentType::Artillery) {
      combatSupportRegimentTypes.push_back(type);
    }
  }
  std::vector<CombatRegimentType> motorizedCombatSupportRegimentTypes;
  for (auto type : availableRegimentTypes) {
    if (type == CombatRegimentType::MotorizedAntiAir ||
        type == CombatRegimentType::MotorizedAntiTank ||
        type == CombatRegimentType::MotorizedArtillery) {
      combatSupportRegimentTypes.push_back(type);
    }
  }

  std::vector<DivisionTemplate> divisionTemplates;
  for (auto type : desiredTypes) {
    if (type == DivisionType::Infantry) {
      int totalRegiments = RandNum::getRandom(4, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      auto templ = createInfantryTemplate(totalRegiments, columnAmount);
      templ.name = "Infantry Division";
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::Militia) {
      int totalRegiments = RandNum::getRandom(4, 6);
      int columnAmount = RandNum::getRandom(2, 3);
      auto templ = createInfantryTemplate(totalRegiments, columnAmount);
      templ.name = "Militia Division";
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::SupportedInfantry) {
      int totalRegiments = RandNum::getRandom(6, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      int maximumSupportRegiments = RandNum::getRandom(1, 3);
      auto templ = createInfantryTemplate(totalRegiments, columnAmount);
      templ.name = "Supported Infantry Division";
      templ.type = DivisionType::SupportedInfantry;
      addSupportRegiments(templ, availableSupportRegimentTypes,
                          maximumSupportRegiments);
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::HeavyArtilleryInfantry) {
      int totalRegiments = RandNum::getRandom(4, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      auto templ = createInfantryTemplate(totalRegiments, columnAmount);
      templ.name = "Artillery Division";
      templ.type = DivisionType::HeavyArtilleryInfantry;
      int maximumArtilleryRegiments = RandNum::getRandom(1, 4);
      addArtilleryRegiments(templ, availableRegimentTypes,
                            maximumArtilleryRegiments);
      // we may also add support
      int maximumSupportRegiments = RandNum::getRandom(1, 3);
      addSupportRegiments(templ, availableSupportRegimentTypes,
                          maximumSupportRegiments);
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::Cavalry) {
      int totalRegiments = RandNum::getRandom(4, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      auto templ = createCavalryTemplate(totalRegiments, columnAmount);
      templ.name = "Cavalry Division";
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::Motorized) {
      // simple uniform motorized infantry
      int totalRegiments = RandNum::getRandom(4, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      auto templ = createMotorizedTemplate(totalRegiments, columnAmount);
      templ.name = "Motorized Division";
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::SupportedMotorized) {
      int totalRegiments = RandNum::getRandom(4, 8);
      int columnAmount = RandNum::getRandom(2, 4);
      int maximumSupportRegiments = RandNum::getRandom(1, 3);
      auto templ = createMotorizedTemplate(totalRegiments, columnAmount);
      templ.type = DivisionType::SupportedMotorized;
      templ.name = "Supported Motorized Division";
      addSupportRegiments(templ, availableSupportRegimentTypes,
                          maximumSupportRegiments);
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::HeavyArtilleryMotorized) {
      int totalRegiments = RandNum::getRandom(4, 8);
      int columnAmount = RandNum::getRandom(2, 4);
      int maximumArtilleryRegiments = RandNum::getRandom(1, 3);
      auto templ = createMotorizedTemplate(totalRegiments, columnAmount);
      templ.type = DivisionType::HeavyArtilleryMotorized;
      templ.name = "Motorized Artillery Division";
      addArtilleryRegiments(templ, availableRegimentTypes,
                            maximumArtilleryRegiments);
      // we may also add support
      int maximumSupportRegiments = RandNum::getRandom(1, 3);
      addSupportRegiments(templ, availableSupportRegimentTypes,
                          maximumSupportRegiments);
      divisionTemplates.push_back(templ);
    } else if (type == DivisionType::Armor) {
    }
  }
  // calc cost for all templates
  for (auto &templ : divisionTemplates) {
    templ.cost = calculateDivisionCost(templ);
  }

  return divisionTemplates;
}
} // namespace Scenario::Hoi4