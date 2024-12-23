#include "generic/NameGenerator.h"

namespace Scenario {
namespace NameGeneration {
// Function to get a random letter based on weights
std::string getRandomLetter(const std::vector<std::string> &letters,
                            const std::map<std::string, float> &weights) {
  // Calculate cumulative weights manually
  std::vector<float> cumulativeWeights;
  float currentSum = 0.0f;
  for (const auto &letter : letters) {
    currentSum += weights.at(letter);
    cumulativeWeights.push_back(currentSum);
  }

  // Generate a random number in the range [0, total weight)
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(
      0.0, cumulativeWeights.empty() ? 0.0f : cumulativeWeights.back());

  float randomWeight = dis(gen);

  // Find the letter corresponding to the random weight
  auto it = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(),
                             randomWeight);
  if (it == cumulativeWeights.end()) {
    return ""; // Return an empty string if no match is found
  }
  return letters[std::distance(cumulativeWeights.begin(), it)];
}

// Function to generate a hard token
std::string generateHardToken(const std::vector<std::string> &letters,
                              std::map<std::string, float> alphabet,
                              std::set<std::string> &existingTokens) {
  std::string token;
  do {
    token.clear();
    int tokenLength = RandNum::getRandom(2, 3);
    for (int i = 0; i < tokenLength; i++) {
      token += getRandomLetter(letters, alphabet);
    }
  } while (existingTokens.find(token) != existingTokens.end());
  existingTokens.insert(token);
  return token;
}

// Function to generate a soft token
std::string generateSoftToken(const std::vector<std::string> &letters,
                              std::map<std::string, float> alphabet,
                              std::set<std::string> &existingTokens) {
  std::string token;
  do {
    token.clear();
    int tokenLength = 2;
    for (int i = 0; i < tokenLength; i++) {
      std::string letter = getRandomLetter(letters, alphabet);
      // Ensure that only very rarely may the same letter be used twice in a
      // soft token if it has a length of 2
      if (i == 1 && token[0] == letter[0] &&
          RandNum::getRandom(0, 100) > 5) {
        // If the same letter is chosen and the random number is greater than 5,
        // choose a different letter
        do {
          letter = getRandomLetter(letters, alphabet);
        } while (token[0] == letter[0]);
      }
      token += letter;
    }
  } while (existingTokens.find(token) != existingTokens.end());
  existingTokens.insert(token);
  return token;
}

std::string generateName(NameData &nameData) {
  auto selectedRule{
      nameData
          .nameRules[RandNum::getRandom((size_t)0, nameData.nameRules.size())]};
  auto selectedRuleNum{Fwg::Parsing::getTokens(selectedRule, ';')};
  std::string name{Detail::getToken(selectedRuleNum, nameData)};
  // insert before the toupper, to ensure we don't add the same name twice
  nameData.disallowedTokens.insert(name);
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

  do {
    int offset = std::clamp(retries - 1, 0, (int)name.size() - 3);
    tag = name.substr(0 + offset, 3);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
  } while (nameData.disallowedTokens.find(tag) !=
               nameData.disallowedTokens.end() &&
           retries++ < 10);
  if (retries == 11) {
    tag = Detail::getRandomMapElement("consonants", nameData.groups) +
          Detail::getRandomMapElement("consonants", nameData.groups) +
          Detail::getRandomMapElement("consonants", nameData.groups);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
  }
  nameData.disallowedTokens.insert(tag);
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
    Fwg::Parsing::Scenario::replaceOccurences(stateName, "templateAdj",
                                              adjective);
  else
    Fwg::Parsing::Scenario::replaceOccurences(stateName, "template", name);
  return stateName;
}

std::string generateCityName(NameData &nameData) {
  return generateName(nameData);
}

NameData prepare(const std::string &path, const std::string &gamePath) {
  Fwg::Utils::Logging::logLine("Preparing name generation from path", path);
  NameData nameData;

  auto additionalForbidden = Fwg::Parsing::getLines(
      Fwg::Cfg::Values().resourcePath + "hoi4//history//forbidden_tags.txt");
  for (const auto &tag : additionalForbidden)
    nameData.disallowedTokens.insert(tag);

  if (std::filesystem::exists(path)) {
    nameData.nameRules = Fwg::Parsing::getLines(path + "//name_rules.txt");
    Detail::readMap(path + "//token_groups.txt", nameData.groups);
    Detail::readMap(path + "//state_types.txt", nameData.ideologyNames);
    Detail::readMap(path + "//faction_names.txt", nameData.factionNames);
    if (gamePath.size() && std::filesystem::exists(gamePath)) {
      const auto forbiddenTags = ResourceLoading::loadForbiddenTags(gamePath);
      for (const auto &tag : forbiddenTags)
        nameData.disallowedTokens.insert(tag);
    }
  } else {
    Fwg::Utils::Logging::logLine(
        "ERROR: Can't prepare namedata as path does not exist!");
  }

  //for (auto i = 0; i < 10; i++) {
  //  LanguageGroup languageGroup;
  //  languageGroup.generate(5);
  //}

  return nameData;
}
namespace Detail {
void readMap(const std::string path,
             std::map<std::string, std::vector<std::string>> &map) {
  auto groupLines{Fwg::Parsing::getLines(path)};
  for (const auto &line : groupLines) {
    auto tokens = Fwg::Parsing::getTokens(line, ';');
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
  }
}

std::string getToken(const std::vector<std::string> &rule,
                     const NameData &nameData) {
  std::string retString{""};
  do {

    for (auto i = 0; i < rule.size(); i++)
      retString += getRandomMapElement(rule[i], nameData.groups);
  } while (nameData.disallowedTokens.find(retString) !=
           nameData.disallowedTokens.end());
  return retString;
}
} // namespace Detail
} // namespace NameGeneration
} // namespace Scenario