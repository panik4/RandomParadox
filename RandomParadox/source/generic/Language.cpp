#include "generic/Language.h"
namespace Scenario {
void Language::vary() {
  for (auto &letter : alphabet) {
    letter.second = RandNum::getRandom(alphabet.at(letter.first) * 0.8,
                                       alphabet.at(letter.first) * 1.2);
  }
  double sum = 0;
  for (const auto &letter : alphabet) {
    sum += letter.second;
  }
  for (auto &letter : alphabet) {
    letter.second /= sum;
  }
}

void Language::fillAllLists() {
  articles.clear();
  citySuffixes.clear();
  cityPrefixes.clear();
  cityNames.clear();
  firstNames.clear();
  lastNames.clear();
  names.clear();
  shipNames.clear();
  airplaneNames.clear();

  for (int i = 0; i < 3; i++) {
    std::string article;
    bool hasVowel = false;
    int reqSize = 2 + rand() % 2;
    for (int j = 0; j < reqSize; j++) {
      std::string letter = "";
      if ((!hasVowel && j == reqSize - 1) || rand() % reqSize == 0) {
        letter = getRandomLetter(vowels, alphabet);
        hasVowel = true;
      } else {
        letter = getRandomLetter(consonants, alphabet);
      }
      article += letter;
    }
    if (!hasVowel) {
      article[rand() % article.size()] = getRandomLetter(vowels, alphabet)[0];
    }
    articles.push_back(article);
  }

  for (int i = 0; i < 2; i++) {
    citySuffixes.push_back(generateWord(2));
    citySuffixes.push_back(generateWord(3));
  }
  for (int i = 0; i < 2; i++) {
    cityPrefixes.push_back(generateWord(2));
    cityPrefixes.push_back(generateWord(3));
  }
  bool articlesUsed = false;
  if (rand() % 3 == 0) {
    articlesUsed = true;
  }
  std::string prefixSeparator = " ";
  if (rand() % 3 == 0) {
    prefixSeparator = "-";
  }
  for (auto &cityPrefix : cityPrefixes) {
    cityPrefix[0] = toupper(cityPrefix[0]);
  }
  for (auto &article : articles) {
    article[0] = toupper(article[0]);
  }
  port = generateWord(2 + rand() % 3);

  for (int i = 0; i < 1000; i++) {
    std::string cityName;
    if (rand() % 3 == 0) {
      if (articlesUsed && rand() % 2 == 0) {
        cityName += Fwg::Utils::selectRandom(articles);
        cityName += " ";
      } else {
        cityName += Fwg::Utils::selectRandom(cityPrefixes);
        cityName += prefixSeparator;
      }
    }
    auto coreName = generateGenericWord();
    coreName[0] = toupper(coreName[0]);
    cityName += coreName;
    if (rand() % 3 == 0) {
      auto suffix = Fwg::Utils::selectRandom(citySuffixes);
      if (rand() % 3 == 0) {
        cityName += " ";
        suffix[0] = toupper(suffix[0]);
      }
      cityName += suffix;
    }
    cityNames.push_back(cityName);
  }

  std::set<std::string> usedFirstNames;
  for (int i = 0; i < 1000; i++) {
    std::string firstName = generateGenericCapitalizedWord();
    if (usedFirstNames.find(firstName) == usedFirstNames.end()) {
      firstNames.push_back(firstName);
      usedFirstNames.insert(firstName);
    }
  }
  std::set<std::string> usedLastNames;
  for (int i = 0; i < 1000; i++) {
    std::string lastName = generateGenericCapitalizedWord();
    if (usedLastNames.find(lastName) == usedLastNames.end() &&
        usedFirstNames.find(lastName) == usedFirstNames.end()) {
      lastNames.push_back(lastName);
      usedLastNames.insert(lastName);
    }
  }
  std::set<std::string> usedNames;
  for (int i = 0; i < 1000; i++) {
    std::string shipName = generateGenericCapitalizedWord();
    if (usedNames.find(shipName) == usedNames.end()) {
      shipNames.push_back(shipName);
      usedNames.insert(shipName);
    }
    std::string airplaneName = generateGenericCapitalizedWord();
    if (usedNames.find(airplaneName) == usedNames.end()) {
      airplaneNames.push_back(airplaneName);
      usedNames.insert(airplaneName);
    }
  }
}

std::string Language::generateWord(const std::vector<std::string> &tokenSet) {
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

std::string Language::generateWord(int tokenSetLength) {
  std::vector<std::vector<std::string>> tokenSetsWithLength;
  for (const auto &tokenSet : tokenSets) {
    if (tokenSet.size() == tokenSetLength) {
      tokenSetsWithLength.push_back(tokenSet);
    }
  }
  std::vector<std::string> tokenSet =
      Fwg::Utils::selectRandom(tokenSetsWithLength);
  return generateWord(tokenSet);
}

std::string Language::generateGenericWord() {
  std::vector<std::string> tokenSet = Fwg::Utils::selectRandom(tokenSets);
  return generateWord(tokenSet);
}

std::string Language::generateGenericCapitalizedWord() {
  std::vector<std::string> tokenSet = Fwg::Utils::selectRandom(tokenSets);
  std::string word = generateWord(tokenSet);
  word[0] = toupper(word[0]);
  return word;
}
} // namespace Scenario