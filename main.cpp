#include "UI/GUI.h"
#include "utils/Logging.h"
#include "utils/Startup.h"

#include <filesystem>

int main() {
  Fwg::Utils::Startup::Configuration startup;
  try {
    Fwg::Utils::Startup::loadConfiguration(startup,
                                           "configs/RandomParadox.json");
    Fwg::Utils::Logging::logLine("Creating the exports folder");
    std::filesystem::create_directory(startup.workingDirectory + "exports//");

    GUI gui;
    Fwg::Utils::Logging::logLine("Starting the GUI");
    const int result = gui.shiny(startup.applicationConfig.value(),
                                 startup.configDirectory, startup.username);
    Fwg::Utils::Logging::logLine("Exited the GUI");
    Fwg::Utils::Startup::writeDiagnosticDump("", startup.configDirectory);
    Fwg::Utils::Logging::logLine("Done with the generation");
    return result;
  } catch (const std::exception &e) {
    Fwg::Utils::Logging::logLine("Application startup/runtime failed: ",
                                 e.what());
    Fwg::Utils::Startup::writeDiagnosticDump(e.what(), startup.configDirectory);
    return -1;
  }
}
