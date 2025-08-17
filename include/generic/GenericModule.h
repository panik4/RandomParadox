#pragma once
#include "countries/CountryGen.h"
#include "generic/ModGenerator.h"
#include "parsing/ParserUtils.h"
#include "utils/Logging.h"
#include "utils/RpxUtils.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <string>
namespace Rpx {
class GenericModule {

protected:
  bool cut;

public:
  std::shared_ptr<Rpx::ModGenerator> generator;


};
} // namespace Rpx