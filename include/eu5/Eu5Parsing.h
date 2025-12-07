#include "eu5/Eu5Generator.h"
#include "parsing/ParserUtils.h"
#include <areas/ArdaProvince.h>
#include <areas/ArdaRegion.h>

namespace Rpx::Eu5::Parsing {
namespace Writing {
void writeMetadata(const std::string &path);
void mainMenuOverrides(const std::string &path);
void overwrites(const std::string &path);

} // namespace Writing

namespace History {
}

}; // namespace Rpx::Eu5::Parsing