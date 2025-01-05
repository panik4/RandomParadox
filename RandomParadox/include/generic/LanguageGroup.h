#pragma once
#include "Language.h"
#include "RandNum.h"
#include "utils/Utils.h"
#include <algorithm>
#include <iostream>
#include <map>
namespace Scenario {
class LanguageGroup {
  std::string name;
  // between 0.2 and 0.6
  double vowelShare;
  // weigh each of the letters in the alphabet, preinitialized with all
  // letters
  std::map<std::string, float> alphabet = {
      {"a", 0.0}, {"b", 0.0}, {"c", 0.0}, {"d", 0.0}, {"e", 0.0}, {"f", 0.0},
      {"g", 0.0}, {"h", 0.0}, {"i", 0.0}, {"j", 0.0}, {"k", 0.0}, {"l", 0.0},
      {"m", 0.0}, {"n", 0.0}, {"o", 0.0}, {"p", 0.0}, {"q", 0.0}, {"r", 0.0},
      {"s", 0.0}, {"t", 0.0}, {"u", 0.0}, {"v", 0.0}, {"w", 0.0}, {"x", 0.0},
      {"y", 0.0}, {"z", 0.0}};
  std::vector<std::string> consonants;
  std::vector<std::string> vowels;

  // tokens
  std::set<std::string> hardTokens;
  std::set<std::string> softTokens;

  std::set<std::string> startTokens;
  std::set<std::string> middleTokens;
  std::set<std::string> endTokens;
  std::set<std::string> allStartTokens = {"ch", "sh", "tr", "str", "qu", "ph",
                                          "sw", "fr", "pl", "gr",  "cl", "fl",
                                          "sp", "dr", "bl", "gl",  "st"};

  std::set<std::string> allMiddleTokens = {
      "ch", "sh", "rt",  "tr", "lt", "rb", "br", "str", "ng", "qu", "nd", "sr",
      "rd", "ss", "cht", "mb", "cc", "nm", "pp", "rr",  "tt", "nz", "bw", "rl",
      "ts", "lk", "sk",  "sp", "pt", "ks", "gn", "ft",  "mp", "rtz"};

  std::set<std::string> allEndTokens = {"rt", "lt", "ng", "nd", "rd", "ss",
                                        "pp", "sk", "mp", "ft", "rk", "ld"};

public:
  std::vector<std::shared_ptr<Language>> languages;
  // rulesets for generating names, with keys to describe where to draw from
  std::vector<std::vector<std::string>> tokenSets;

  // Function to find the likeliest tokens based on the likelihood of the
  // tokens' letters in the language group alphabet
  std::vector<std::string>
  findLikeliestTokens(const std::set<std::string> &tokens,
                      const std::map<std::string, float> &alphabet, int count);

  void generateTokenSets();
  void generate(int languageAmount);

};
} // namespace Scenario