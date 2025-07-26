
#include "hoi4/goals/Effects.h"
namespace Rpx::Hoi4::Effects {
void constructEffects(std::vector<EffectGrouping> &effectGroupings,
                      std::shared_ptr<Hoi4Country> &country) {
  // randomly select one of the effectGroupings
  auto effectGrouping = Fwg::Utils::selectRandom(effectGroupings);
  // we need to clear the effectGroupings, and then add the new effects
  effectGroupings.clear();
  for (auto &effect : effectGrouping.effects) {
    auto &name = effect.name;
    auto &parameters = effect.parameters;
    std::vector<std::string> newParameters;
    if (parameters.size() == 0) {
      // no parameters, we can keep the effect as is
    } else if (parameters.size() > 1) {
      Fwg::Utils::Logging::logLine("More than one parameter for effect " +
                                   name);
    } else {
      // we split the parameters by | if we have more than one in one (), such
      // as with resource and amount eff:develop_random_resource(anyRes|4%40);
      auto subParams = Fwg::Parsing::getTokens(parameters[0], '|');
      for (int i = 0; i < subParams.size(); i++) {
        auto &subParam = subParams[i];
        parameters.resize(i + 1);
        // we have a list of potential parameters
        if (subParam.find(",") != std::string::npos) {
          // split the parameter by ,
          auto parts = Fwg::Parsing::getTokens(subParam, ',');
          for (auto &part : parts) {
            newParameters.push_back(part);
          }
          // we not determine randomly which parameter to use
          auto &selectedParameter = Fwg::Utils::selectRandom(newParameters);
          // now we overwrite it
          subParam = selectedParameter;
        }
        // we have a range, and need to take one of the values
        else if (subParam.find("%") != std::string::npos) {
          // split by %
          auto parts = Fwg::Parsing::getTokens(subParam, '%');
          // we have a range
          if (parts.size() == 2) {
            auto min = std::stod(parts[0]);
            auto max = std::stod(parts[1]);
            // try to detect if this is a floating point number or an integer
            if (min == static_cast<int>(min) && max == static_cast<int>(max)) {
              auto selectedValue = RandNum::getRandom<int>(min, max);
              subParam = std::to_string(selectedValue);
            } else {
              auto selectedValue = RandNum::getRandom<double>(min, max);
              subParam = std::to_string(selectedValue);
            }
          } else {
            Fwg::Utils::Logging::logLine("Invalid range for effect " + name);
          }
        } else {
          // we have a single parameter, and we can keep it
          if (subParam == "anyRes") {
            const std::array<std::string, 6> resources{
                "oil", "steel", "tungsten", "aluminium", "rubber", "chromium"};
            // we need to select a random resource
            auto selectedResource = Fwg::Utils::selectRandom(resources);
            subParam = selectedResource;
          } else if (subParam == "faction_name") {
            subParam = country->tag + "faction";
          }
        }
        parameters[i] = subParam;
      }
    }
  }
  effectGroupings.push_back(effectGrouping);
}
} // namespace Rpx::Hoi4::Effects
