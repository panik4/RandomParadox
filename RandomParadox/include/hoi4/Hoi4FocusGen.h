
#include "hoi4/Hoi4Country.h"
#include "hoi4/NationalFocus.h"

namespace Scenario::Hoi4 {
namespace FocusGen {
// build a focus from the chain
NationalFocus buildFocus(const std::vector<std::string> chainStep,
                         const Hoi4Country &source, const Hoi4Country &target);

void mergeFocusBranches(Hoi4Country &source);

// make a tree out of all focus chains and single foci
void buildFocusTree(Hoi4Country &source);
// check if a national focus fulfills requirements
bool stepFulfillsRequirements(
    const std::string stepRequirements,
    const std::vector<std::set<Hoi4Country>> &stepTargets);
// check if a national focus fulfills requirements
bool targetFulfillsRequirements(
    const std::string &targetRequirements, const Hoi4Country &source,
    const Hoi4Country &target,
    const std::vector<std::set<std::string>> &levelTargets,
    const std::vector<std::shared_ptr<Scenario::Region>> &gameRegions,
    const int level);
// evaluate the focus chains for each country
void evaluateCountryGoals(
    std::map<std::string, Hoi4Country> &hoi4Countries,
    const std::vector<std::shared_ptr<Scenario::Region>> &gameRegions);
}; // namespace FocusGen
} // namespace Scenario::Hoi4