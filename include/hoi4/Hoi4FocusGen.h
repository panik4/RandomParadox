
#include "hoi4/Hoi4Country.h"
#include "hoi4/goals/GoalGeneration.h"
#include "parsing/ParserUtils.h"

namespace Rpx::Hoi4 {
namespace FocusGen {

std::string addAvailableBlocks(std::shared_ptr<Hoi4Country> country,
                   std::shared_ptr<Goal> goal,
                   const std::map<std::string, std::string> &availableMap);

// add bypss blocks
std::string
addBypassBlocks(std::shared_ptr<Hoi4Country> country,
                std::shared_ptr<Goal> goal,
                const std::map<std::string, std::string> &bypassMap);

// evaluate the focus chains for each country
void evaluateCountryGoals(
    std::vector<std::shared_ptr<Hoi4Country>> &hoi4Countries,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions);

}; // namespace FocusGen
} // namespace Rpx::Hoi4