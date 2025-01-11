#include "generic/CultureGroup.h"

void Scenario::CultureGroup::determineVisualType() {
  // placeholder, select one of the VisualTypes randomly
  visualType = static_cast<VisualType>(rand() % 5);
}