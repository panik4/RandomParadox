#include "generic/ParserUtils.h"
#include "vic3/Vic3Utils.h"

namespace Scenario::Vic3::Importing {
std::map<std::string, Technology> readTechs(const std::string &inFolder);
std::map<std::string, ProductionmethodGroup> readProdMethodGroups(
    const std::string &inFolder,
    const std::map<std::string, Productionmethod> &productionmethods);
std::map<std::string, Productionmethod>
readProdMethods(const std::string &inFolder,
                const std::map<std::string, Good> &goods);
std::vector<BuildingType> readBuildings(
    const std::string &inFolder,
    std::map<std::string, ProductionmethodGroup> productionmethodGroups);
std::map<std::string, Buypackage>
readBuypackages(const std::string &inFolder,
                const std::map<std::string, PopNeed> &popNeeds);
std::map<std::string, PopNeed>
readPopNeeds(const std::string &inFolder,
             const std::map<std::string, Good> &goods);
std::map<std::string, Good> readGoods(const std::string &inFolder);

} // namespace Scenario::Vic3::Importing