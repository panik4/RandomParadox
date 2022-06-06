#include "generic/ParserUtils.h"
#include <generic/GameProvince.h>

class Eu4Parser {
  using pU = ParserUtils;

public:
  static void dumpAdj(const std::string path,
                      const std::vector<GameProvince> &provinces);
  static void writeDefaultMap(const std::string path,
                              const std::vector<GameProvince> &provinces);
};