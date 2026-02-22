#include "areas/ArdaRegion.h"
#include "hoi4/Hoi4Utils.h"
#include "parsing/ParserUtils.h"
namespace Rpx::Hoi4 {
namespace DecisionGen {

void generateResourceDecisions(
    Rpx::Hoi4::DecisionData &decisionData,
    const int numDecisions,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions);

void generateDecisions(
    Rpx::Hoi4::DecisionData &decisionData,
    const std::vector<std::shared_ptr<Arda::ArdaRegion>> &ardaRegions);


}; // namespace DecisionGen
} // namespace Rpx::Hoi4