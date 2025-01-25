
#include "generic/LanguageGroup.h"
namespace Scenario {
// std::string
// generateHardToken(const std::vector<std::string> &consonants,
//                                  const std::map<std::string, float>
//                                  &alphabet, const std::set<std::string>
//                                  &existingTokens) {
//   std::string token;
//   do {
//     token = consonants[RandNum::getRandom<int>(0, consonants.size() - 1)];
//   } while (existingTokens.find(token) != existingTokens.end());
//   return token;
// }
//
// std::string
// generateSoftToken(const std::vector<std::string> &vowels,
//                                  const std::map<std::string, float>
//                                  &alphabet, const std::set<std::string>
//                                  &existingTokens) {
//   std::string token;
//   do {
//     token = vowels[RandNum::getRandom<int>(0, vowels.size() - 1)];
//   } while (existingTokens.find(token) != existingTokens.end());
//   return token;
// }

std::vector<std::string>
LanguageGroup::findLikeliestTokens(const std::set<std::string> &tokens,
                                   const std::map<std::string, float> &alphabet,
                                   int count) {
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

void LanguageGroup::generateTokenSets() {
  for (int i = 0; i < 6; i++) {
    std::vector<std::string> tokenSet;

    bool lastWasHardOrConsonant = RandNum::getRandom<bool>();

    int setLength = 2 + i / 2;
    bool hadStartToken = false;
    bool hadMiddleToken = false;
    bool hadEndToken = false;

    for (int j = 0; j < setLength; j++) {
      if (lastWasHardOrConsonant) {
        if (rand() % 2 == 0) {
          tokenSet.push_back("vowel");
        } else {
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

    for (const auto &token : tokenSet) {
      std::cout << token << " ";
    }
    std::cout << "\n";
    tokenSets.push_back(tokenSet);
  }
  std::cout << "\n";
}
void LanguageGroup::generate(int languageAmount) {
  vowelShare = RandNum::getRandom(0.2, 0.6);

  for (auto &letter : alphabet) {
    letter.second = RandNum::getRandom(0.01, 1.0);
  }

  consonants = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n",
                "p", "q", "r", "s", "t", "v", "w", "x", "y", "z"};
  vowels = {"a", "e", "i", "o", "u"};

  for (const auto &vowel : vowels) {
    alphabet.at(vowel) = RandNum::getRandom(0.5, 1.0) * vowelShare;
  }

  double sum = 0;
  for (const auto &letter : alphabet) {
    sum += letter.second;
  }
  for (auto &letter : alphabet) {
    letter.second /= sum;
  }

  for (int i = 0; i < 1; i++) {
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

  for (int i = 0; i < 0; i++) {
    startTokens.insert(Fwg::Utils::selectRandom(hardTokens));
    middleTokens.insert(Fwg::Utils::selectRandom(hardTokens));
    endTokens.insert(Fwg::Utils::selectRandom(hardTokens));
  }

  generateTokenSets();

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
    languages.push_back(std::make_shared<Language>(language));
  }
}
} // namespace Scenario