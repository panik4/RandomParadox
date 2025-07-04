#pragma once
#pragma once
#include "Language.h"
#include "LanguageGroup.h"
#include "ParserUtils.h"
#include "ResourceLoading.h"
#include "ScenarioUtils.h"
#include <map>
#include <string>
#include <vector>
namespace Scenario {
namespace NameGeneration {

struct NameData {
  // containers
  std::set<std::string> tags;
  std::map<std::string, std::vector<std::string>> ideologyNames;
  std::map<std::string, std::vector<std::string>> factionNames;
  std::set<std::string> disallowedTokens;
};
// member functions
std::string generateTag(const std::string name, NameData &nameData);
std::string generateFactionName(const std::string &ideology,
                                const std::string name,
                                const std::string adjective,
                                const NameData &nameData);
std::string modifyWithIdeology(const std::string &ideology,
                               const std::string name,
                               const std::string adjective,
                               const NameData &nameData);
NameData prepare(const std::string &path, const std::string &gamePath, const GameType gameType);
namespace Detail {
void readMap(const std::string path,
             std::map<std::string, std::vector<std::string>> &map);
std::string
getRandomMapElement(const std::string key,
                    const std::map<std::string, std::vector<std::string>> map);
} // namespace Detail
}; // namespace NameGeneration
} // namespace Scenario