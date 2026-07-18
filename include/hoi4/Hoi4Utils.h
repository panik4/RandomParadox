#pragma once
#include <array>
#include <map>
#include <string>
#include <vector>
#include "utils/Archive.h"
namespace Rpx::Hoi4 {
struct Faction {
  std::string name = "";
  Arda::Utils::Ideology ideology = Arda::Utils::Ideology::NEUTRALITY;
  std::string faction_template = "";
  std::string factionLeader = "";
  std::vector<std::string> memberTags;

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    ar &name;
    ar.serialiseEnum(ideology);
    ar &faction_template &factionLeader &memberTags;
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) { serialise(ar); }
};


struct DecisionData {
  std::map<std::string, std::string> decisionNames;
  std::vector<std::string> resourceDecisions;

  void serialise(Fwg::Utils::Serialisation::Archive &ar) {
    ar &decisionNames &resourceDecisions;
  }
  void deserialise(Fwg::Utils::Serialisation::Archive &ar) { serialise(ar); }
};

} // namespace Rpx::Hoi4