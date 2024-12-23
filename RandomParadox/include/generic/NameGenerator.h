#pragma once
#pragma once
#include "ParserUtils.h"
#include "ResourceLoading.h"
#include <map>
#include <string>
#include <vector>
namespace Scenario {
namespace NameGeneration {
// Function to get a random letter based on weights
std::string getRandomLetter(const std::vector<std::string> &letters,
                            const std::map<std::string, float> &weights);

// Function to generate a hard token
std::string generateHardToken(const std::vector<std::string> &letters,
                              std::map<std::string, float> alphabet,
                              std::set<std::string> &existingTokens);

// Function to generate a soft token
std::string generateSoftToken(const std::vector<std::string> &letters,
                              std::map<std::string, float> alphabet,
                              std::set<std::string> &existingTokens);

struct Language {
  std::string name;
  std::vector<std::string> citySuffixes;
  std::vector<std::string> firstNames;
  std::vector<std::string> lastNames;
  // usually a variation of the language group alphabet
  std::map<std::string, float> alphabet;
  // separate grouping
  std::vector<std::string> consonants;
  std::vector<std::string> vowels;
  // tokens
  std::set<std::string> hardTokens;
  std::set<std::string> softTokens;

  std::set<std::string> startTokens;
  std::set<std::string> middleTokens;
  std::set<std::string> endTokens;
  // rulesets for generating names, with keys to describe where to draw from
  std::vector<std::vector<std::string>> tokenSets;

  // add slight variations to the language, by changing the weights of the
  // letters in the alphabet, and then randomly replace some of the letters in
  // the tokens
  void vary() {
    // slightly modify the weighting of each letter in the alphabet
    for (auto &letter : alphabet) {
      letter.second = RandNum::getRandom(alphabet.at(letter.first) * 0.8,
                                         alphabet.at(letter.first) * 1.2);
    }
    // normalize the weights
    double sum = 0;
    for (const auto &letter : alphabet) {
      sum += letter.second;
    }
    for (auto &letter : alphabet) {
      letter.second /= sum;
    }
  }

  void generateName() {}
  std::string generateGenericWord() {
    // now generate a generic word, by randomly selecting from the token sets
    std::vector<std::string> tokenSet = Fwg::Utils::selectRandom(tokenSets);
    std::string word;
    for (const auto &token : tokenSet) {
      if (token == "startToken") {
        word += Fwg::Utils::selectRandom(startTokens);
      } else if (token == "middleToken") {
        word += Fwg::Utils::selectRandom(middleTokens);
      } else if (token == "endToken") {
        word += Fwg::Utils::selectRandom(endTokens);
      } else if (token == "consonant") {
        word += getRandomLetter(consonants, alphabet);
      } else if (token == "vowel") {
        word += getRandomLetter(vowels, alphabet);
      } else if (token == "softToken") {
        word += Fwg::Utils::selectRandom(softTokens);
      }
    }
    return word;
  }
};

struct LanguageGroup {
  std::string name;
  // between 0.2 and 0.6
  double vowelShare;
  std::vector<Language> languages;
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
  // rulesets for generating names, with keys to describe where to draw from
  std::vector<std::vector<std::string>> tokenSets;

  // Function to find the likeliest tokens based on the likelihood of the
  // tokens' letters in the language group alphabet
  std::vector<std::string>
  findLikeliestTokens(const std::set<std::string> &tokens,
                      const std::map<std::string, float> &alphabet, int count) {
    std::vector<std::pair<std::string, float>> tokenLikelihoods;

    for (const auto &token : tokens) {
      float tokenLikelihood = 1.0;
      for (const auto &letter : token) {
        std::string letterStr(1, letter);
        tokenLikelihood *= alphabet.at(letterStr);
      }
      tokenLikelihoods.emplace_back(token, tokenLikelihood);
    }

    // Sort tokens by likelihood in descending order
    std::sort(tokenLikelihoods.begin(), tokenLikelihoods.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });

    // Collect the top 'count' likeliest tokens
    std::vector<std::string> likeliestTokens;
    for (int i = 0; i < count && i < tokenLikelihoods.size(); ++i) {
      likeliestTokens.push_back(tokenLikelihoods[i].first);
    }

    return likeliestTokens;
  }

  void generateTokenSets() {
    for (int i = 0; i < 12; i++) {
      std::vector<std::string> tokenSet;

      // Ensure proper sequence: consonant/hard -> vowel/soft ->
      // consonant/hard
      bool lastWasHardOrConsonant =
          RandNum::getRandom<bool>(); // Assume the sequence starts with
                                      // consonant/hard

      int setLength = 2 + i / 2;
      bool hadStartToken = false;
      bool hadMiddleToken = false;
      bool hadEndToken = false;

      // Generate a sequence of tokens
      for (int j = 0; j < setLength;
           j++) { // Arbitrary length of tokens in the set
        if (lastWasHardOrConsonant) {
          // Choose a vowel or a soft token
          if (!vowels.empty() && (softTokens.empty() || rand() % 5 == 0)) {
            tokenSet.push_back("vowel");
          } else if (!softTokens.empty()) {
            tokenSet.push_back("softToken");
          }
          lastWasHardOrConsonant = false;
        } else {
          if (rand() % 4 == 0) {
            tokenSet.push_back("consonant");
          } else {
            if (!startTokens.empty() && !hadStartToken &&
                ((setLength < 3 && j + 1 < setLength) || j + 2 < setLength)) {
              tokenSet.push_back("startToken");
              hadStartToken = true;
            } else if (!middleTokens.empty() && !hadMiddleToken &&
                       j + 1 < setLength) {
              tokenSet.push_back("middleToken");
              hadMiddleToken = true;
            } else if (!endTokens.empty() && !hadEndToken) {
              tokenSet.push_back("endToken");
              hadEndToken = true;
            }
          }

          lastWasHardOrConsonant = true;
        }
      }

      // print the tokenset to one line
      for (const auto &token : tokenSet) {
        std::cout << token << " ";
      }
      std::cout << "\n";
      tokenSets.push_back(tokenSet);
    }
    std::cout << "\n";
  }
  void generate(int languageAmount) {
    // randomly set the vowel share
    vowelShare = RandNum::getRandom(0.2, 0.6);
    // randomly give every letter of the alphabet a weight
    for (auto &letter : alphabet) {
      letter.second = RandNum::getRandom(0.01, 1.0);
    }
    // initialize the vowels and consonants
    consonants = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n",
                  "p", "q", "r", "s", "t", "v", "w", "x", "y", "z"};
    vowels = {"a", "e", "i", "o", "u"};

    // ensure that the vowels have a higher weight
    for (const auto &vowel : vowels) {
      alphabet.at(vowel) = RandNum::getRandom(0.5, 1.0) * vowelShare;
    }

    // normalize the weights
    double sum = 0;
    for (const auto &letter : alphabet) {
      sum += letter.second;
    }
    for (auto &letter : alphabet) {
      letter.second /= sum;
    }
    // generate additional completely random basic hard- and softtokens
    for (int i = 0; i < 10; i++) {
      hardTokens.insert(generateHardToken(consonants, alphabet, hardTokens));
      softTokens.insert(generateSoftToken(vowels, alphabet, softTokens));
    }

    // add randomly to start-, middle- and endtokens, from the predefined sets
    for (int i = 0; i < 1; i++) {
      // Add the 5 likeliest tokens to start-, middle- and endtokens, from the
      // predefined sets
      auto likeliestStartTokens =
          findLikeliestTokens(allStartTokens, alphabet, 5);
      auto likeliestMiddleTokens =
          findLikeliestTokens(allMiddleTokens, alphabet, 5);
      auto likeliestEndTokens = findLikeliestTokens(allEndTokens, alphabet, 5);

      startTokens.insert(likeliestStartTokens.begin(),
                         likeliestStartTokens.end());
      middleTokens.insert(likeliestMiddleTokens.begin(),
                          likeliestMiddleTokens.end());
      endTokens.insert(likeliestEndTokens.begin(), likeliestEndTokens.end());
    }

    // generate the start-, middle- and endtokens, by randomly selecting from
    // the hard- and softtokens
    for (int i = 0; i < 0; i++) {
      startTokens.insert(Fwg::Utils::selectRandom(hardTokens));
      middleTokens.insert(Fwg::Utils::selectRandom(hardTokens));
      endTokens.insert(Fwg::Utils::selectRandom(hardTokens));
    }
    // generate the token sets
    generateTokenSets();

    // generate the languages
    for (int i = 0; i < languageAmount; i++) {
      Language language;
      language.alphabet = alphabet;
      language.consonants = consonants;
      language.vowels = vowels;
      language.hardTokens = hardTokens;
      language.softTokens = softTokens;
      language.startTokens = startTokens;
      language.middleTokens = middleTokens;
      language.endTokens = endTokens;
      language.tokenSets = tokenSets;
      language.vary();
      language.generateName();
      for (int i = 0; i < 10; i++) {

        std::cout << language.generateGenericWord() << std::endl;
      }
      std::cout << "\n\n";
      languages.push_back(language);
    }
  }
};

struct NameData {
  // containers
  std::set<std::string> tags;
  std::vector<std::string> nameRules;
  std::map<std::string, std::vector<std::string>> groups;
  std::map<std::string, std::vector<std::string>> ideologyNames;
  std::map<std::string, std::vector<std::string>> factionNames;
  std::set<std::string> disallowedTokens;
};
// member functions
std::string generateName(NameData &nameData);
std::string generateAdjective(const std::string &name,
                              const NameData &nameData);
std::string generateTag(const std::string name, NameData &nameData);
std::string generateFactionName(const std::string &ideology,
                                const std::string name,
                                const std::string adjective,
                                const NameData &nameData);
std::string modifyWithIdeology(const std::string &ideology,
                               const std::string name,
                               const std::string adjective,
                               const NameData &nameData);
std::string generateCityName(NameData &nameData);
NameData prepare(const std::string &path, const std::string &gamePath = "");
namespace Detail {

std::string getToken(const std::vector<std::string> &rule,
                     const NameData &nameData);
void readMap(const std::string path,
             std::map<std::string, std::vector<std::string>> &map);
std::string
getRandomMapElement(const std::string key,
                    const std::map<std::string, std::vector<std::string>> map);
} // namespace Detail
}; // namespace NameGeneration
} // namespace Scenario