#include "NationalFocus.h"
#include "Hoi4Parser.h"
int NationalFocus::IDcounter = 0;
std::map<std::string, NationalFocus::FocusType> NationalFocus::typeMapping;
std::map<std::string, std::string> NationalFocus::availableMap;
std::map<std::string, std::string> NationalFocus::bypassMap;
std::map<std::string, std::string> NationalFocus::rewardMap;
NationalFocus::NationalFocus() {}

NationalFocus::NationalFocus(FocusType fType, bool defaultV, std::string source,
                             std::string dest, std::vector<int> date)
    : ID{ID}, fType{fType}, defaultV{defaultV}, sourceTag{source},
      destTag{dest}, date{date} {
  this->ID = IDcounter++;
  position = {5, 5};
}

NationalFocus::~NationalFocus() {}

void NationalFocus::buildMaps() {
  auto types = Hoi4Parser::readTypeMap();
  for (int i = 0; i < types.size(); i++) {
    typeMapping[types[i]] = (FocusType)i;
  }
  availableMap = Hoi4Parser::readRewardMap(
      "resources\\hoi4\\ai\\national_focus\\baseFiles\\available.txt");
  bypassMap = Hoi4Parser::readRewardMap(
      "resources\\hoi4\\ai\\national_focus\\baseFiles\\bypass.txt");
  rewardMap = Hoi4Parser::readRewardMap(
      "resources\\hoi4\\ai\\national_focus\\baseFiles\\completionRewards.txt");
}

std::ostream &operator<<(std::ostream &os, const NationalFocus &focus) {
  auto action = "";
  switch (focus.fType) {
  case NationalFocus::FocusType::attack: {
    action = " attacks ";
    break;
  }
  case NationalFocus::FocusType::ally: {
    action = " tries to ally ";
    break;
  }
  }
  os << focus.sourceTag << action << focus.destTag << std::endl;
  os << "Alternatives: " << focus.alternativeFoci.size()
     << "; Preceding Foci: " << focus.precedingFoci.size();
  return os;
}
