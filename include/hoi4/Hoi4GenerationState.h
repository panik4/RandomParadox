#pragma once
#include "utils/FwgGenerationState.h"

namespace Rpx::Hoi4 {
struct Hoi4GenerationState {
  Fwg::Availability economy = Fwg::Availability::Unavailable;
  Fwg::Availability resources = Fwg::Availability::Unavailable;
  Fwg::Availability countries = Fwg::Availability::Unavailable;
  template <class Archive> void serialize(Archive &ar, unsigned int) {
    ar & economy & resources & countries;
  }
};
} // namespace Rpx::Hoi4
