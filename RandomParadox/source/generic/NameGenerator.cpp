#include "generic/NameGenerator.h"

namespace Scenario {
namespace NameGeneration {

std::string generateTag(const std::string name, NameData &nameData) {
  std::string tag{""};
  int retries = 0;

  do {
    int offset = std::clamp(retries - 1, 0, (int)name.size() - 3);
    tag = name.substr(0 + offset, 3);
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
    if (tag.size() < 3)
      tag += "X";
    std::cout << "offset: " << offset << " tag: " << tag << std::endl;
  } while (nameData.disallowedTokens.find(tag) !=
               nameData.disallowedTokens.end() &&
           retries++ < 10);
  if (retries >= 10) {
    std::vector<std::string> letters{"A", "B", "C", "D", "E", "F",
                                     "G", "H", "I", "J", "K", "L"};
    do {
      // add a random letter to the tag
      tag.resize(3);

      tag[2] = Fwg::Utils::selectRandom(letters)[0];
    } while (nameData.disallowedTokens.find(tag) !=
                 nameData.disallowedTokens.end() &&
             retries++ < 20);
  }
  if (tag.size() != 3)
        throw(std::exception(std::string("Incorrect tag size in generating tag " + tag).c_str()));
  if (retries >= 20)
    throw(std::exception(
        std::string("Too many tries generating tag " + tag).c_str()));

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

NameData prepare(const std::string &path, const std::string &gamePath) {
  Fwg::Utils::Logging::logLine("Preparing name generation from path", path);
  NameData nameData;

  auto additionalForbidden = Fwg::Parsing::getLines(
      Fwg::Cfg::Values().resourcePath + "hoi4//history//forbidden_tags.txt");
  for (const auto &tag : additionalForbidden)
    nameData.disallowedTokens.insert(tag);

  if (std::filesystem::exists(path)) {
    Detail::readMap(path + "//state_types.txt", nameData.ideologyNames);
    Detail::readMap(path + "//faction_names.txt", nameData.factionNames);
    try {

      if (gamePath.size() && std::filesystem::exists(gamePath)) {
        const auto forbiddenTags = ResourceLoading::loadForbiddenTags(gamePath);
        for (const auto &tag : forbiddenTags)
          nameData.disallowedTokens.insert(tag);
      }
    } catch (std::exception e) {
      Fwg::Utils::Logging::logLine("ERROR: Path to game does not exist");
    }
  } else {
    Fwg::Utils::Logging::logLine(
        "ERROR: Can't prepare namedata as path does not exist!");
  }

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

} // namespace Detail
} // namespace NameGeneration
} // namespace Scenario