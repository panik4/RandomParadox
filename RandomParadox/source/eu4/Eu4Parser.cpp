#include "eu4/Eu4Parser.h"

void Eu4Parser::dumpAdj(const std::string path,
                        const std::vector<GameProvince> &provinces) {
  Logger::logLine("Eu4 Parser: Map: Writing Adjacencies");
  // From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment
  // empty file for now
  std::string content;
  content.append("From;To;Type;Through;start_x;start_y;stop_x;stop_y;Comment\n");
    const auto &randProv = provinces[0];
  content.append(std::to_string(randProv.ID));
  content.append(";");
  content.append(std::to_string(randProv.ID));
  content.append(";");
  content.append("land");
  content.append(";");
  content.append(std::to_string(randProv.ID));
  content.append(";-1;-1;-1;-1;Filler\n");
  content.append("-1;-1;;-1;-1;-1;-1;-1;-1;");
  pU::writeFile(path, content);
}
void Eu4Parser::writeDefaultMap(const std::string path,
                                const std::vector<GameProvince> &provinces) {
  Logger::logLine("Eu4 Parser: Map: Writing default map");
  auto content = pU::readFile("resources\\eu4\\map\\default.map");
  pU::replaceOccurences(content, "templateWidth",
                        std::to_string(Env::Instance().width));
  pU::replaceOccurences(content, "templateHeight",
                        std::to_string(Env::Instance().height));
  pU::replaceOccurences(content, "templateProvinces",
                        std::to_string(provinces.size()));
  std::string seaStarts{""};
  std::string lakes{""};
  for (const auto &province : provinces) {
    if (province.baseProvince->sea) {
      seaStarts.append(std::to_string(province.ID) + " ");
      if (seaStarts.size() % 76 < 10 && seaStarts.size() >= 10)
        seaStarts.append("\n\t\t\t\t");
    } else if (province.baseProvince->isLake) {
      lakes.append(std::to_string(province.ID) + " ");
      if (lakes.size() % 76 < 10 && lakes.size() >= 10)
        lakes.append("\n\t\t\t");
    }
  }
  pU::replaceOccurences(content, "templateSeaStarts", seaStarts);
  pU::replaceOccurences(content, "templateLakes", lakes);
  pU::writeFile(path, content);
}
