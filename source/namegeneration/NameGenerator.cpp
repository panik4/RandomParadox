#include "namegeneration/NameGenerator.h"

namespace Rpx {
namespace NameGeneration {

std::string generateTag(const std::string name, NameData &nameData) {
  std::string tag{""};
  int retries = 0;
  // all letters in the alphabet
  const std::vector<std::string> letters{
      "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
      "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

  do {
    tag = name.substr(0, std::min<int>(3, name.size()));
    std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
    if (tag.size() < 3)
      tag += Fwg::Utils::selectRandom(letters)[0];
    // if we have a retry, simply replace one of the letters with a random one
    if (retries > 0)
      tag[RandNum::getRandom(0, 2)] = Fwg::Utils::selectRandom(letters)[0];
  } while (nameData.disallowedTokens.find(tag) !=
               nameData.disallowedTokens.end() &&
           retries++ < 10);
  if (retries >= 10) {
    do {
      // add a random letter to the tag
      tag.resize(3);
      tag[2] = Fwg::Utils::selectRandom(letters)[0];
    } while (nameData.disallowedTokens.find(tag) !=
                 nameData.disallowedTokens.end() &&
             retries++ < 20);
  }
  if (tag.size() != 3) {
    std::cerr << "Incorrect tag size" << std::endl;
    throw(std::exception(
        std::string("Incorrect tag size in generating tag " + tag).c_str()));
  }
  if (retries >= 20)
    throw(std::exception(
        std::string("Too many tries generating tag " + tag).c_str()));

  nameData.disallowedTokens.insert(tag);
  if (tag == "AUX")
    std::cout << "WTF" << std::endl;

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
    Rpx::Parsing::replaceOccurences(stateName, "templateAdj",
                                              adjective);
  else
    Rpx::Parsing::replaceOccurences(stateName, "template", name);
  return stateName;
}

NameData prepare(const std::string &path, const std::string &gamePath, const GameType gameType) {
  Fwg::Utils::Logging::logLine("Preparing name generation from path: ", path);
  NameData nameData;

  auto additionalForbidden = Fwg::Parsing::getLines(
      Fwg::Cfg::Values().resourcePath + "hoi4//history//forbidden_tags.txt");
  for (const auto &tag : additionalForbidden)
    nameData.disallowedTokens.insert(tag);

  if (gameType == GameType::Hoi4 && std::filesystem::exists(path)) {
    Detail::readMap(path + "//state_types.txt", nameData.ideologyNames);
    Detail::readMap(path + "//faction_names.txt", nameData.factionNames);
    try {

      if (gamePath.size() && std::filesystem::exists(gamePath)) {
        const auto forbiddenTags = ResourceLoading::loadForbiddenTags(gamePath);
        for (const auto &tag : forbiddenTags)
          nameData.disallowedTokens.insert(tag);
      }
    } catch (std::exception e) {
      Fwg::Utils::Logging::logLine("ERROR: Path to game does not exist",
                                   e.what());
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
} // namespace Rpx