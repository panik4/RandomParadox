#include "NameGenerator.h"

std::vector<std::string> NameGenerator::nameRules;
std::map<std::string, std::vector<std::string>> NameGenerator::groups;
std::map<std::string, std::vector<std::string>> NameGenerator::ideologyNames;

std::string NameGenerator::generateName() {
  auto selectedRule{
      nameRules[Data::getInstance().random2() % nameRules.size()]};
  auto selectedRuleNum{ParserUtils::getTokens(selectedRule, ';')};
  std::string name{getToken(selectedRuleNum)};
  std::transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);
  return name;
}

std::string NameGenerator::generateAdjective(std::string &name) {
  for (auto vowel : groups["vowels"])
    if (vowel.front() == name.back())
      return name + getRandomMapElement("adjModifierVowel", groups);
  return name + getRandomMapElement("adjModifierConsonant", groups);
}

std::string NameGenerator::generateTag(std::string name,
                                       std::set<std::string> &tags) {
  std::string tag = "";
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

std::string NameGenerator::getRandomMapElement(
    std::string key, std::map<std::string, std::vector<std::string>> map) {
  try {
    return *UtilLib::select_random(map[key]);
  } catch (std::exception e) {
    auto str = "Error in Name Generation. Make sure the key: \"" + key +
               "\" of the namegroup or token group is present";
    throw(std::exception(str.c_str()));
    return "";
  }
}

std::string NameGenerator::getToken(std::vector<std::string> &rule) {
  std::string retString = "";
  for (int i = 0; i < rule.size(); i++) {
    retString += getRandomMapElement(rule[i], groups);
  }
  return retString;
}

std::string NameGenerator::modifyWithIdeology(const std::string &ideology,
                                              std::string name,
                                              std::string adjective) {
  auto stateName = getRandomMapElement(ideology, ideologyNames);
  if (stateName.find("templateAdj") != std::string::npos)
    ParserUtils::replaceOccurences(stateName, "templateAdj", adjective);
  else
    ParserUtils::replaceOccurences(stateName, "template", name);
  return stateName;
}

void NameGenerator::readMap(
    std::string path, std::map<std::string, std::vector<std::string>> &map) {
  auto groupLines = ParserUtils::getLines(path);
  for (auto &line : groupLines) {
    auto tokens = ParserUtils::getTokens(line, ';');
    for (int i = 1; i < tokens.size(); i++)
      map[tokens[0]].push_back(tokens[i]);
  }
}

void NameGenerator::prepare() {
  nameRules = ParserUtils::getLines("resources\\names\\name_rules.txt");
  readMap("resources\\names\\token_groups.txt", groups);
  readMap("resources\\names\\state_types.txt", ideologyNames);
}
