#include "hoi4/Hoi4FormatConverter.h"
namespace Scenario::Gfx::Hoi4 {
using namespace Scenario::Gfx::Textures;
using namespace Fwg;
using namespace Fwg::Gfx;

FormatConverter::FormatConverter(const std::string &gamePath,
                                 const std::string &gameTag)
    : Scenario::Gfx::FormatConverter(gamePath, gameTag) {}

FormatConverter::~FormatConverter() {}
} // namespace Scenario::Gfx::Hoi4