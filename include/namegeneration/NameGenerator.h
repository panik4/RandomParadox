#pragma once
#pragma once
#include "culture/NameUtils.h"
#include "hoi4/Hoi4Utils.h"
#include "io/GameDataImporter.h"
#include "language/Language.h"
#include "parsing/ParserUtils.h"
#include "utils/ArdaUtils.h"
#include "utils/RpxUtils.h"
#include <map>
#include <string>
#include <vector>
namespace Rpx {
namespace NameGeneration {

// member functions
std::string generateFactionName(const Arda::Utils::Ideology &ideology,
                                const std::string name,
                                const std::string adjective,
                                const Arda::Names::NameData &nameData);
std::string modifyWithIdeology(const Arda::Utils::Ideology &ideology,
                               const std::string name,
                               const std::string adjective,
                               const Arda::Names::NameData &nameData);
Arda::Names::NameData prepare(const std::string &path,
                              const std::string &gamePath,
                              const GameType gameType);
namespace Detail {
void readMap(const std::string path,
             std::map<Arda::Utils::Ideology, std::vector<std::string>> &map);
std::string getRandomMapElement(
    const Arda::Utils::Ideology key,
    const std::map<Arda::Utils::Ideology, std::vector<std::string>> map);
} // namespace Detail
}; // namespace NameGeneration
} // namespace Rpx