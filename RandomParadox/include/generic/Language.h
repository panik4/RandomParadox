#pragma once
#include "RandNum.h"
#include "utils/Utils.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>
namespace Scenario {
// Function to get a random letter based on weights
static std::string getRandomLetter(const std::vector<std::string> &letters,
                            const std::map<std::string, float> &weights) {
  // Calculate cumulative weights manually
  std::vector<float> cumulativeWeights;
  float currentSum = 0.0f;
  for (const auto &letter : letters) {
    currentSum += weights.at(letter);
    cumulativeWeights.push_back(currentSum);
  }

  // Generate a random number in the range [0, total weight)
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(
      0.0, cumulativeWeights.empty() ? 0.0f : cumulativeWeights.back());

  float randomWeight = dis(gen);

  // Find the letter corresponding to the random weight
  auto it = std::lower_bound(cumulativeWeights.begin(), cumulativeWeights.end(),
                             randomWeight);
  if (it == cumulativeWeights.end()) {
    return ""; // Return an empty string if no match is found
  }
  return letters[std::distance(cumulativeWeights.begin(), it)];
}

// Function to generate a hard token
static std::string generateHardToken(const std::vector<std::string> &letters,
                              std::map<std::string, float> alphabet,
                              std::set<std::string> &existingTokens) {
  std::string token;
  do {
    token.clear();
    int tokenLength = RandNum::getRandom(2, 3);
    for (int i = 0; i < tokenLength; i++) {
      token += getRandomLetter(letters, alphabet);
    }
  } while (existingTokens.find(token) != existingTokens.end());
  existingTokens.insert(token);
  return token;
}

// Function to generate a soft token
static std::string generateSoftToken(const std::vector<std::string> &letters,
                              std::map<std::string, float> alphabet,
                              std::set<std::string> &existingTokens) {
  std::string token;
  do {
    token.clear();
    int tokenLength = 2;
    for (int i = 0; i < tokenLength; i++) {
      std::string letter = getRandomLetter(letters, alphabet);
      // Ensure that only very rarely may the same letter be used twice in a
      // soft token if it has a length of 2
      if (i == 1 && token[0] == letter[0] && RandNum::getRandom(0, 100) > 5) {
        // If the same letter is chosen and the random number is greater than 5,
        // choose a different letter
        do {
          letter = getRandomLetter(letters, alphabet);
        } while (token[0] == letter[0]);
      }
      token += letter;
    }
  } while (existingTokens.find(token) != existingTokens.end());
  existingTokens.insert(token);
  return token;
}


class Language {
public:
  std::string name;
  std::vector<std::string> articles; // like the, la, le, der, die, das
  std::string
      port; // like Port, Puerto, Porto, Haven, can be either prefix or suffix

  std::vector<std::string> cityPrefixes; // like Bad, New, Saint, San, Los, Las,
                                         // El, La, but randomly generated
  std::vector<std::string> citySuffixes; // like ville, city, town, burg, but
                                         // randomly generated
  std::vector<std::string> cityNames;
  std::vector<std::string> portNames;
  // std::vector<std::string> mountainCityNames;
  // std::vector<std::string> valleyCityNames;
  // std::vector<std::string> riverCityNames; // should follow patterns like

  std::vector<std::string> firstNames;
  std::vector<std::string> lastNames;
  std::vector<std::string> names;

  std::vector<std::string> shipNames;
  std::vector<std::string> airplaneNames;

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
  void vary();

  void fillAllLists();

  std::string generateWord(const std::vector<std::string> &tokenSet);

  // method to generate a word, but using only a random tokenset of specified
  // length
  std::string generateWord(int tokenSetLength);

  std::string generateGenericWord();
  std::string generateGenericCapitalizedWord();
};
} // namespace Scenario