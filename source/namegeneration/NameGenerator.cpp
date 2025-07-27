#include "namegeneration/NameGenerator.h"

namespace Rpx {
namespace NameGeneration {


std::string generateFactionName(const std::string &ideology,
                                const std::string name,
                                const std::string adjective,
                                const Arda::Names::NameData &nameData) {
  return Detail::getRandomMapElement(ideology, nameData.factionNames);
}

std::string modifyWithIdeology(const std::string &ideology,
                               const std::string name,
                               const std::string adjective,
                               const Arda::Names::NameData &nameData) {
  auto stateName{Detail::getRandomMapElement(ideology, nameData.ideologyNames)};
  if (stateName.find("templateAdj") != std::string::npos)
    Rpx::Parsing::replaceOccurences(stateName, "templateAdj",
                                              adjective);
  else
    Rpx::Parsing::replaceOccurences(stateName, "template", name);
  return stateName;
}

Arda::Names::NameData prepare(const std::string &path, const std::string &gamePath, const GameType gameType) {
  Fwg::Utils::Logging::logLine("Preparing name generation from path: ", path);
  Arda::Names::NameData nameData;

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