#include "hoi4/Hoi4Army.h"
namespace Scenario::Hoi4 {

DivisionTemplate createInfantryTemplate(int totalRegiments, int &columns,
                                        const RegimentType &type) {
  DivisionTemplate divisionTemplate;
  divisionTemplate.type = DivisionType::Infantry;
  // if we have more than 8 regiments, we need at least three columns if we
  // havent this many alredy

  if (totalRegiments > 8) {
    columns = std::max<int>(columns, 3);
  }

  int regimentAmount = totalRegiments / columns;
  int leftOverRegiments = totalRegiments % columns;
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

DivisionTemplate createCavalryTemplate(int totalRegiments, int columns) {
  return DivisionTemplate();
}

DivisionTemplate createMotorizedTemplate(int totalRegiments, int columns) {
  return DivisionTemplate();
}

DivisionTemplate createArmorTemplate(int totalRegiments, int columns) {
  return DivisionTemplate();
}

DivisionTemplate addSupportRegiments(
    DivisionTemplate &divisionTemplate,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes) {
  return divisionTemplate;
}

DivisionTemplate addArtilleryRegiments(
    DivisionTemplate &divisionTemplate,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes) {
  return divisionTemplate;
}

std::vector<DivisionTemplate> Scenario::Hoi4::createDivisionTemplates(
    const std::vector<DivisionType> &desiredTypes,
    const std::vector<RegimentType> &availableRegimentTypes,
    const std::vector<SupportRegimentType> &availableSupportRegimentTypes) {
  std::vector<DivisionTemplate> divisionTemplates;
  for (auto type : desiredTypes) {
    if (type == DivisionType::Infantry) {
      int totalRegiments = RandNum::getRandom(4, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      divisionTemplates.push_back(
          createInfantryTemplate(9, columnAmount, RegimentType::Infantry));
    } else if (type == DivisionType::Militia) {
      // return DivisionTemplate();
    } else if (type == DivisionType::SupportedInfantry) {
      int totalRegiments = RandNum::getRandom(4, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      auto templ = createInfantryTemplate(totalRegiments, columnAmount,
                                          RegimentType::Infantry);
      divisionTemplates.push_back(
          addSupportRegiments(templ, availableSupportRegimentTypes));
    } else if (type == DivisionType::HeavyArtilleryInfantry) {
      int totalRegiments = RandNum::getRandom(4, 12);
      int columnAmount = RandNum::getRandom(2, 4);
      auto templ = createInfantryTemplate(totalRegiments, columnAmount,
                                          RegimentType::Infantry);
      templ = addArtilleryRegiments(templ, availableSupportRegimentTypes);
      // we may also add support
      divisionTemplates.push_back(
          addSupportRegiments(templ, availableSupportRegimentTypes));
    } else if (type == DivisionType::Cavalry) {
    } else if (type == DivisionType::Motorized) {
    } else if (type == DivisionType::Armor) {
    }
  }
  return divisionTemplates;
}
} // namespace Scenario::Hoi4