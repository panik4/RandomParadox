#pragma once
#pragma once
#include "language/Language.h"
#include "language/LanguageGroup.h"
#include "parsing/ParserUtils.h"
#include "io/GameDataImporter.h"
#include "utils/RpxUtils.h"
#include "culture/NameUtils.h"
#include <map>
#include <string>
#include <vector>
namespace Rpx {
namespace NameGeneration {


// member functions
std::string generateFactionName(const std::string &ideology,
                                const std::string name,
                                const std::string adjective,
                                const Arda::Names::NameData &nameData);
std::string modifyWithIdeology(const std::string &ideology,
                               const std::string name,
                               const std::string adjective,
                               const Arda::Names::NameData &nameData);
Arda::Names::NameData prepare(const std::string &path,
                              const std::string &gamePath,
                              const GameType gameType);
namespace Detail {
void readMap(const std::string path,
             std::map<std::string, std::vector<std::string>> &map);
std::string
getRandomMapElement(const std::string key,
                    const std::map<std::string, std::vector<std::string>> map);
} // namespace Detail
}; // namespace NameGeneration
} // namespace Rpx