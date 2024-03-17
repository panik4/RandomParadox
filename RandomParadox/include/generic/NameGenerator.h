#pragma once
#pragma once
#include "ParserUtils.h"
#include "ResourceLoading.h"
#include <map>
#include <string>
#include <vector>
namespace Scenario {
namespace NameGeneration {
struct NameData {
  // containers
  std::set<std::string> tags;
  std::vector<std::string> nameRules;
  std::map<std::string, std::vector<std::string>> groups;
  std::map<std::string, std::vector<std::string>> ideologyNames;
  std::map<std::string, std::vector<std::string>> factionNames;
  std::set<std::string> disallowedTokens;
};
// member functions
std::string generateName(NameData &nameData);
std::string generateAdjective(const std::string &name,
                              const NameData &nameData);
std::string generateTag(const std::string name, 
                        NameData &nameData);
std::string generateFactionName(const std::string &ideology,
                                const std::string name,
                                const std::string adjective,
                                const NameData &nameData);
std::string modifyWithIdeology(const std::string &ideology,
                               const std::string name,
                               const std::string adjective,
                               const NameData &nameData);
NameData prepare(const std::string &path, const std::string &gamePath = "");
namespace Detail {

std::string getToken(const std::vector<std::string> &rule,
                     const NameData &nameData);
void readMap(const std::string path,
             std::map<std::string, std::vector<std::string>> &map);
std::string
getRandomMapElement(const std::string key,
                    const std::map<std::string, std::vector<std::string>> map);
}
}; // namespace NameGeneration
} // namespace Scenario