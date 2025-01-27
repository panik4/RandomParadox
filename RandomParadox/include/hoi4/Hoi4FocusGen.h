
#include "hoi4/Hoi4Country.h"
#include "hoi4/NationalFocus.h"
#include "hoi4/goals/GoalGeneration.h"

namespace Scenario::Hoi4 {
namespace FocusGen {


// build a focus from the chain
NationalFocus buildFocus(const std::vector<std::string> chainStep,
                         const Hoi4Country &source, const Hoi4Country &target);

void mergeFocusBranches(Hoi4Country &source);

// make a tree out of all focus chains and single foci
void buildFocusTree(Hoi4Country &source);

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
    const std::vector<std::shared_ptr<Scenario::Region>> &gameRegions);

}; // namespace FocusGen
} // namespace Scenario::Hoi4