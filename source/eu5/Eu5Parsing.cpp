#include "eu5/Eu5Parsing.h"
using namespace Fwg;
namespace Logging = Fwg::Utils::Logging;
namespace pU = Fwg::Parsing;
namespace Rpx::Eu5::Parsing {
namespace Writing {

void writeMetadata(const std::string &path) {
  Fwg::Utils::Logging::logLine("Vic3 Parser: Mod: Writing metadata.json");
  const auto templateFile =
      pU::readFile(Fwg::Cfg::Values().resourcePath + "eu5//metadata.json");

  pU::writeFile(path, templateFile);
}

void mainMenuOverrides(const std::string &path) {
  Fwg::Utils::Logging::logLine("Eu5 Parser: Mod: Writing main menu overrides");
  auto resourcePath = Fwg::Cfg::Values().resourcePath + "eu5/main_menu";
  std::filesystem::copy(resourcePath, path,
                        std::filesystem::copy_options::recursive |
                            std::filesystem::copy_options::overwrite_existing);
}

void overwrites(const std::string &path) {
  Fwg::Utils::Logging::logLine(
      "Eu5 Parser: Mod: Writing some file replacements");
  // lets copy in the overwrites file
  auto resourcePath = Fwg::Cfg::Values().resourcePath + "eu5/";
  std::vector<std::string> overwriteFolders = {"gfx/map/locators_override",
                                               "gfx/map/map_objects"};
  for (const auto &folder : overwriteFolders) {
    Fwg::Utils::Logging::logLine("Eu5 Parser: Mod: Writing overwrites in " +
                                 folder);
    std::filesystem::copy(
        resourcePath + folder, path + folder,
        std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing);
  }
}

} // namespace Writing

namespace Reading {}
} // namespace Rpx::Eu5::Parsing
