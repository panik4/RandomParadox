#pragma once
#include <array>
#include <map>
#include <string>
#include <vector>
#include "utils/SerialisationFwd.h"
namespace Rpx::Hoi4 {
struct Faction {
  std::string name = "";
  Arda::Utils::Ideology ideology = Arda::Utils::Ideology::NEUTRALITY;
  std::string faction_template = "";
  std::string factionLeader = "";
  std::vector<std::string> memberTags;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/) {
    ar &name &ideology &faction_template &factionLeader &memberTags;
  }
};


struct DecisionData {
  std::map<std::string, std::string> decisionNames;
  std::vector<std::string> resourceDecisions;

  template<class Archive>
  void serialize(Archive &ar, const unsigned int /*version*/) {
    ar &decisionNames &resourceDecisions;
  }
};

} // namespace Rpx::Hoi4