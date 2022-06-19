#pragma once
#include "ParserUtils.h"
#include <map>
#include <string>
#include <vector>
namespace Scenario {
class NameGenerator {
  using PU = ParserUtils;
  // containers
  static std::vector<std::string> nameRules;
  static std::map<std::string, std::vector<std::string>> groups;
  static std::map<std::string, std::vector<std::string>> ideologyNames;
  static std::map<std::string, std::vector<std::string>> factionNames;

public:
  // member functions
  static std::string generateName();
  static std::string generateAdjective(const std::string &name);
  static std::string generateTag(const std::string name,
                                 std::set<std::string> &tags);
  static std::string generateFactionName(const std::string &ideology,
                                         const std::string name,
                                         const std::string adjective);
  static std::string getRandomMapElement(
      const std::string key,
      const std::map<std::string, std::vector<std::string>> map);
  static std::string getToken(const std::vector<std::string> &rule);
  static std::string modifyWithIdeology(const std::string &ideology,
                                        const std::string name,
                                        const std::string adjective);
  static void readMap(const std::string path,
                      std::map<std::string, std::vector<std::string>> &map);
  static void prepare();
};
} // namespace Scenario