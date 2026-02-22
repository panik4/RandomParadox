#pragma once
#include <array>
#include <map>
#include <string>
#include <vector>
namespace Rpx::Hoi4 {
struct Faction {
  std::string name = "";
  Arda::Utils::Ideology ideology = Arda::Utils::Ideology::NEUTRALITY;
  std::string faction_template = "";
  std::string factionLeader = "";
  std::vector<std::string> memberTags;
};


struct DecisionData {
  std::map<std::string, std::string> decisionNames;
  std::vector<std::string> resourceDecisions;


};

} // namespace Rpx::Hoi4