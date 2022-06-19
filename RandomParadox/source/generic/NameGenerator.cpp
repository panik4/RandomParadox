#include "generic/NameGenerator.h"

namespace Scenario {
std::vector<std::string> NameGenerator::nameRules;
std::map<std::string, std::vector<std::string>> NameGenerator::groups;
std::map<std::string, std::vector<std::string>> NameGenerator::ideologyNames;
std::map<std::string, std::vector<std::string>> NameGenerator::factionNames;

std::string NameGenerator::generateName() {
  auto selectedRule{nameRules[RandNum::randNum() % nameRules.size()]};
  auto selectedRuleNum{PU::getTokens(selectedRule, ';')};
  std::string name{getToken(selectedRuleNum)};
  std::transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);
  return name;
}

std::string NameGenerator::generateAdjective(const std::string &name) {
  for (const auto &vowel : groups["vowels"])
    if (vowel.front() == name.back())
      return name + getRandomMapElement("adjModifierVowel", groups);
  return name + getRandomMapElement("adjModifierConsonant", groups);
}

std::string NameGenerator::generateTag(const std::string name,
                                       std::set<std::string> &tags) {
  std::string tag{""};
  int retries = 0;
  while ((tag.size() == 0 || tags.find(tag) != tags.end()) && retries++ < 10) {
    int offset = std::clamp(retries - 1, 0, (int)name.size() - 3);
    tag = name.substr(0 + offset, 3);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
  }
  if (retries == 11) {
    tag = getRandomMapElement("consonants", groups) +
          getRandomMapElement("consonants", groups) +
          getRandomMapElement("consonants", groups);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
  }
  tags.insert(tag);
  return tag;
}

std::string NameGenerator::generateFactionName(const std::string &ideology,
                                               const std::string name,
                                               const std::string adjective) {

  return getRandomMapElement(ideology, factionNames);
}

std::string NameGenerator::getRandomMapElement(
    const std::string key,
    const std::map<std::string, std::vector<std::string>> map) {
  try {
    return FastWorldGen::Utils::selectRandom(map.at(key));
  } catch (std::exception e) {
    auto str = "Error in Name Generation. Make sure the key: \"" + key +
               "\" of the namegroup or token group is present";
    throw(std::exception(str.c_str()));
    return "";
  }
}

std::string NameGenerator::getToken(const std::vector<std::string> &rule) {
  std::string retString{""};
  for (auto i = 0; i < rule.size(); i++)
    retString += getRandomMapElement(rule[i], groups);
  return retString;
}

std::string NameGenerator::modifyWithIdeology(const std::string &ideology,
                                              const std::string name,
                                              const std::string adjective) {
  auto stateName{getRandomMapElement(ideology, ideologyNames)};
  if (stateName.find("templateAdj") != std::string::npos)
    PU::replaceOccurences(stateName, "templateAdj", adjective);
  else
    PU::replaceOccurences(stateName, "template", name);
  return stateName;
}

void NameGenerator::readMap(
    const std::string path,
    std::map<std::string, std::vector<std::string>> &map) {
  auto groupLines{PU::getLines(path)};
  for (const auto &line : groupLines) {
    auto tokens = PU::getTokens(line, ';');
    for (int i = 1; i < tokens.size(); i++)
      map[tokens[0]].push_back(tokens[i]);
  }
}

void NameGenerator::prepare() {
  nameRules = PU::getLines("resources\\names\\name_rules.txt");
  readMap("resources\\names\\token_groups.txt", groups);
  readMap("resources\\names\\state_types.txt", ideologyNames);
  readMap("resources\\names\\faction_names.txt", factionNames);
}
} // namespace Scenario