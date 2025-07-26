#include "hoi4/NationalFocus.h"
#include "hoi4/Hoi4Parsing.h"
namespace Rpx::Hoi4 {
int NationalFocus::IDcounter = 0;
std::map<std::string, NationalFocus::FocusType> NationalFocus::typeMapping;
std::map<std::string, std::string> NationalFocus::availableMap;
std::map<std::string, std::string> NationalFocus::bypassMap;
std::map<std::string, std::string> NationalFocus::rewardMap;
NationalFocus::NationalFocus() {}

NationalFocus::NationalFocus(FocusType fType, bool defaultV,
                             const std::string &source, const std::string &dest,
                             const std::vector<int> &date)
    : ID{IDcounter++}, fType{fType}, defaultV{defaultV}, sourceTag{source},
      destTag{dest}, date{date[0], date[1], date[2]}, position{5, 5} {}

NationalFocus::~NationalFocus() {}

void NationalFocus::buildMaps() {
  auto types = Parsing::readTypeMap();
  for (int i = 0; i < types.size(); i++) {
    typeMapping[types[i]] = (FocusType)i;
  }
  availableMap = Parsing::readRewardMap(
      Fwg::Cfg::Values().resourcePath + "hoi4//ai//national_focus//baseFiles//available.txt");
  bypassMap = Parsing::readRewardMap(
      Fwg::Cfg::Values().resourcePath + "hoi4//ai//national_focus//baseFiles//bypass.txt");
  rewardMap = Parsing::readRewardMap(
      Fwg::Cfg::Values().resourcePath + "hoi4//ai//national_focus//baseFiles//completionRewards.txt");
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
  case NationalFocus::FocusType::generic: {
    action = " generic ";
    break;
  }
  }
  os << focus.sourceTag << action << focus.destTag << std::endl;
  os << "Alternatives: " << focus.xorFoci.size()
     << "; Preceding Foci: " << focus.precedingFoci.size();
  return os;
}
const NationalFocus &FocusBranch::operator[](int index) const { return foci[index]; }
size_t FocusBranch::size() { return foci.size(); }

} // namespace Rpx::Hoi4