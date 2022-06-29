#include "generic/NameGenerator.h"

namespace Scenario {
namespace NameGeneration {
std::string generateName(const NameData &nameData) {
  auto selectedRule{
      nameData.nameRules[Fwg::RandNum::randNum() % nameData.nameRules.size()]};
  auto selectedRuleNum{ParserUtils::getTokens(selectedRule, ';')};
  std::string name{Detail::getToken(selectedRuleNum, nameData)};
  std::transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);
  return name;
}

std::string generateAdjective(const std::string &name,
                              const NameData &nameData) {
  for (const auto &vowel : nameData.groups.at("vowels"))
    if (vowel.front() == name.back())
      return name +
             Detail::getRandomMapElement("adjModifierVowel", nameData.groups);
  return name +
         Detail::getRandomMapElement("adjModifierConsonant", nameData.groups);
}

std::string generateTag(const std::string name, NameData &nameData) {
  std::string tag{""};
  int retries = 0;
  while ((tag.size() == 0 || nameData.tags.find(tag) != nameData.tags.end()) &&
         retries++ < 10) {
    int offset = std::clamp(retries - 1, 0, (int)name.size() - 3);
    tag = name.substr(0 + offset, 3);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
  }
  if (retries == 11) {
    tag = Detail::getRandomMapElement("consonants", nameData.groups) +
          Detail::getRandomMapElement("consonants", nameData.groups) +
          Detail::getRandomMapElement("consonants", nameData.groups);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
  }
  nameData.tags.insert(tag);
  return tag;
}

std::string generateFactionName(const std::string &ideology,
                                const std::string name,
                                const std::string adjective,
                                const NameData &nameData) {
  return Detail::getRandomMapElement(ideology, nameData.factionNames);
}

std::string modifyWithIdeology(const std::string &ideology,
                               const std::string name,
                               const std::string adjective,
                               const NameData &nameData) {
  auto stateName{Detail::getRandomMapElement(ideology, nameData.ideologyNames)};
  if (stateName.find("templateAdj") != std::string::npos)
    ParserUtils::replaceOccurences(stateName, "templateAdj", adjective);
  else
    ParserUtils::replaceOccurences(stateName, "template", name);
  return stateName;
}

NameData prepare(const std::string &path, const std::string &gamePath) {
  Fwg::Utils::Logging::logLine("Preparing name generation from path", path);
  NameData nameData;
  nameData.nameRules = ParserUtils::getLines(path + "\\name_rules.txt");
  Detail::readMap(path + "\\token_groups.txt", nameData.groups);
  Detail::readMap(path + "\\state_types.txt", nameData.ideologyNames);
  Detail::readMap(path + "\\faction_names.txt", nameData.factionNames);
  if (gamePath.size()) {
    const auto forbiddenTags = ResourceLoading::loadForbiddenTags(gamePath);
    for (const auto &tag : forbiddenTags)
      nameData.tags.insert(tag);
  }
  return nameData;
}
namespace Detail {
void readMap(const std::string path,
             std::map<std::string, std::vector<std::string>> &map) {
  auto groupLines{ParserUtils::getLines(path)};
  for (const auto &line : groupLines) {
    auto tokens = ParserUtils::getTokens(line, ';');
    for (int i = 1; i < tokens.size(); i++)
      map[tokens[0]].push_back(tokens[i]);
  }
}
std::string
getRandomMapElement(const std::string key,
                    const std::map<std::string, std::vector<std::string>> map) {
  try {
    return Fwg::Utils::selectRandom(map.at(key));
  } catch (std::exception e) {
    auto str = "Error in Name Generation. Make sure the key: \"" + key +
               "\" of the namegroup or token group is present";
    throw(std::exception(str.c_str()));
    return "";
  }
}

std::string getToken(const std::vector<std::string> &rule,
                     const NameData &nameData) {
  std::string retString{""};
  for (auto i = 0; i < rule.size(); i++)
    retString += getRandomMapElement(rule[i], nameData.groups);
  return retString;
}
} // namespace Detail
} // namespace NameGeneration
} // namespace Scenario