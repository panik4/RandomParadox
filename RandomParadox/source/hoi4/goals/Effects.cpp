
#include "hoi4/goals/Effects.h"
namespace Scenario::Hoi4::Effects {
void constructEffects(std::vector<EffectGrouping> &effectGroupings) {
  // randomly select one of the effectGroupings
  auto effectGrouping = Fwg::Utils::selectRandom(effectGroupings);
  // we need to clear the effectGroupings, and then add the new effects
  effectGroupings.clear();
  for (auto &effect : effectGrouping.effects) {
    auto &name = effect.name;
    auto &parameters = effect.parameters;
    std::vector<std::string> newParameters;
    if (parameters.size() > 1) {
      Fwg::Utils::Logging::logLine("More than one parameter for effect " +
                                   name);
    } else {
      // we have a list of potential parameters
      if (parameters[0].find(",") != std::string::npos) {
        // split the parameter by ,
        auto parts = Fwg::Parsing::getTokens(parameters[0], ',');
        for (auto &part : parts) {
          newParameters.push_back(part);
        }
        // we not determine randomly which parameter to use
        auto &selectedParameter = Fwg::Utils::selectRandom(newParameters);
        // now we overwrite it
        parameters[0] = selectedParameter;
      }
      // we have a range, and need to take one of the values
      else if (parameters[0].find("%") != std::string::npos) {
        // split by %
        auto parts = Fwg::Parsing::getTokens(parameters[0], '%');
        // we have a range
        if (parts.size() == 2) {
          auto min = std::stod(parts[0]);
          auto max = std::stod(parts[1]);
          // try to detect if this is a floating point number or an integer
          if (min == static_cast<int>(min) && max == static_cast<int>(max)) {
            auto selectedValue = RandNum::getRandom<int>(min, max);
            parameters[0] = std::to_string(selectedValue);
          } else {
            auto selectedValue = RandNum::getRandom<double>(min, max);
            parameters[0] = std::to_string(selectedValue);
          }
        } else {
          Fwg::Utils::Logging::logLine("Invalid range for effect " + name);
        }
      } else {
        // we have a single parameter, and we can keep it
      }
    }
  }
  effectGroupings.push_back(effectGrouping);
}
} // namespace Scenario::Hoi4::Effects
