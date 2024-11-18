#include "generic/ParserUtils.h"

namespace Fwg::Parsing::Scenario {

void removeCharacter(std::string &content, char character) {
  content.erase(std::remove(content.begin(), content.end(), character),
                content.end());
}
void removeSpecials(std::string &content) {
  removeCharacter(content, '{');
  removeCharacter(content, '\n');
  removeCharacter(content, '\t');
  removeCharacter(content, '\r');
  removeCharacter(content, '=');
  removeCharacter(content, '}');
  // utf-8 BOM stuff
  removeCharacter(content, '�');
  removeCharacter(content, '�');
  removeCharacter(content, '�');
};

std::vector<int> getNumbers(const std::string &content, const char delimiter,
                            const std::set<int> tokensToConvert) {
  bool convertAll = !tokensToConvert.size();
  std::vector<int> numbers{};
  std::stringstream sstream(content);
  std::string token;
  int counter = 0;
  while (std::getline(sstream, token, delimiter)) {
    if (token.size())
      if (convertAll || tokensToConvert.find(counter) != tokensToConvert.end())
        numbers.push_back(stoi(token));
    counter++;
  }
  return numbers;
};

std::vector<int> getNumbersMultiDelim(const std::string &content,
                                      const char delimiter,
                                      const std::set<int> tokensToConvert) {
  std::vector<std::string> delims{"\t"};
  auto cCopy = content;
  for (auto character : delims) {
    replaceOccurences(cCopy, character, " ");
  }
  return getNumbers(cCopy, delimiter, tokensToConvert);
};

std::vector<int> getNumberBlock(std::string content, std::string key) {
  auto bracketBlock = getBracketBlockContent(content, key);
  removeCharacter(bracketBlock, '{');
  removeCharacter(bracketBlock, '\n');
  removeCharacter(bracketBlock, '\t');
  removeCharacter(bracketBlock, '=');
  removeCharacter(bracketBlock, '}');
  replaceOccurences(bracketBlock, key, "");
  return getNumbers(bracketBlock, ' ', std::set<int>{});
}

std::vector<int> getNumberBlockMultiDelim(std::string content,
                                          std::string key) {
  auto bracketBlock = getBracketBlockContent(content, key);
  removeCharacter(bracketBlock, '{');
  removeCharacter(bracketBlock, '\n');
  removeCharacter(bracketBlock, '=');
  removeCharacter(bracketBlock, '}');
  replaceOccurences(bracketBlock, key, "");
  return getNumbersMultiDelim(bracketBlock, ' ', std::set<int>{});
};

bool replaceOccurence(std::string &content, const std::string &key,
                      const std::string &value) {
  size_t pos = content.find(key);
  if (pos != std::string::npos)
    content.replace(pos, key.length(), value);
  return pos != std::string::npos;
}

std::string replaceOccurences(std::string &content, const std::string &key,
                              const std::string &value) {
  while (replaceOccurence(content, key, value)) {
  }
  return content;
};
// replace complete line from beginning of key to linebreak with value
bool replaceLine(std::string &content, const std::string &key,
                 const std::string &value) {
  size_t pos = content.find(key);
  if (pos != std::string::npos) {
    const auto lineEnd = content.find("\n", pos);
    content.replace(pos, lineEnd - pos, value);
  }
  return pos != std::string::npos;
};
// replace complete line from beginning of key to linebreak with value
void replaceLines(std::string &content, const std::string &key,
                  const std::string &value) {
  while (replaceLine(content, key, value)) {
  }
};
bool removeLine(std::string &content, const std::string &key) {
  size_t pos = content.find(key);
  if (pos != std::string::npos) {
    // Find the start of the line
    size_t lineStart = content.rfind("\n", pos);
    lineStart = (lineStart == std::string::npos) ? 0 : lineStart + 1;

    // Find the end of the line
    size_t lineEnd = content.find("\n", pos);
    lineEnd = (lineEnd == std::string::npos) ? content.size() : lineEnd;

    // Erase the line
    content.erase(lineStart, lineEnd - lineStart);
    return true;
  }
  return false;
};

void removeLines(std::string &content, const std::string &key) {
  while (removeLine(content, key)) {
  }
};

// find the closing bracket of a block. Handles opening brackets correctly
// as long as every opening bracket has an opening bracket
size_t findClosingBracket(const std::string &content, size_t startPos) {
  // find opening bracket of this block
  const auto openingBracket = content.find("{", startPos);
  // find next opening bracket
  auto nextOpenBracket = content.find("{", openingBracket + 1);
  // find closing bracket
  auto blockEnd = content.find("}", startPos);
  // found an opening bracket before the closing bracket, means this bracket
  // doesn't close the scope we search
  while (nextOpenBracket != std::string::npos && nextOpenBracket < blockEnd) {
    blockEnd = content.find("}", blockEnd + 1); // find the next closing bracket
    nextOpenBracket =
        content.find("{", nextOpenBracket + 1); // find next opening bracket
  }
  return blockEnd;
}
// reads the bracket block including keyword onwards up until a closing
// bracket
std::string getBracketBlock(const std::string &content, const std::string key) {
  const auto pos = content.find(key);
  if (pos != std::string::npos) {
    const auto blockEnd = findClosingBracket(content, pos) + 1;
    return content.substr(pos, blockEnd - pos);
  }
  return "";
};
// reads the bracket block excluding keyword onwards up until a closing
// bracket
std::string getBracketBlockContent(const std::string &content,
                                   const std::string key) {
  // first get whole block of keyword
  const auto block = getBracketBlock(content, key);
  // now get the opening bracket
  const auto pos = block.find("{") + 1;
  if (pos != std::string::npos) {
    const auto blockEnd = findClosingBracket(block, pos - 1);
    if (blockEnd == -1)
      return "";
    return block.substr(pos, blockEnd - pos);
  }
  return "";
};
// delete the bracket block from the bracket on, leaving the key
bool removeBracketBlockFromBracket(std::string &content,
                                   const std::string key) {
  const auto pos = content.find(key);
  const auto openingBracket = content.find("{", pos);
  if (pos != std::string::npos && openingBracket != std::string::npos) {
    const auto blockEnd = findClosingBracket(content, pos);
    content.erase(openingBracket, blockEnd - openingBracket + 1);
    return true;
  }
  return false;
};
// delete the bracket block from the key on, leaving nothing
bool removeBracketBlockFromKey(std::string &content, const std::string key) {
  const auto pos = content.find(key);
  if (pos != std::string::npos) {
    const auto blockEnd = findClosingBracket(content, pos);
    content.erase(pos, blockEnd - pos + 1);
    return true;
  }
  return false;
};

void removeSurroundingBracketBlock(std::string &content,
                                   const std::string key) {
  auto pos = content.find(key);
  if (pos != std::string::npos) {
    pos = content.rfind("{", pos);
    const auto blockEnd = findClosingBracket(content, pos);
    content.erase(pos, blockEnd - pos);
  }
};
std::string removeSurroundingBracketBlockFromLineBreak(std::string &content,
                                                       const std::string key) {
  auto pos = content.find(key);
  if (pos != std::string::npos) {
    auto pos2 = content.rfind("}", pos);
    pos = content.rfind("{", pos);
    pos = content.rfind("\n", pos);
    auto blockEnd = findClosingBracket(content, pos);
    auto retString = content.substr(pos, blockEnd - pos + 1);
    content.erase(pos, blockEnd - pos + 1);
    return retString;
  }
  return "";
};

std::vector<Block> getOuterBlocks(const std::vector<std::string> &lines) {
  std::vector<Block> outermostBlocks;
  bool insideBlock = false;
  int bracketSurplus = 0;
  Block currentBlock;
  for (auto &line : lines) {
    if (line.find('{') != std::string::npos &&
        line.find('}') != std::string::npos) {
      if (bracketSurplus) {
        currentBlock.content += line + '\n';
        continue;
      } else {
      // ignore this case, don't know how to handle it yet, TODO
        continue;
      }
    }
    if (line.find('{') != std::string::npos) {
      // open a new block here, if we're not in a block
      if (!bracketSurplus && line.find('=') != std::string::npos) {
        insideBlock = true;
        currentBlock.name = line.substr(0, line.find('='));
        // remove potential whitespaces from name
        removeCharacter(currentBlock.name, ' ');
        currentBlock.content.clear();
      } else {
        currentBlock.content += line + '\n';
      }
      bracketSurplus++;
    } else if (insideBlock) {
      if (line.find('}') != std::string::npos) {
        if (bracketSurplus == 1) {
          insideBlock = false;
          outermostBlocks.push_back(currentBlock);
        } else {
          currentBlock.content += line + '\n';
        }
        bracketSurplus--;
      } else {
        currentBlock.content += line + '\n';
      }
    }
  }
  return outermostBlocks;
}

std::string getEntrenched(const std::string &content,
                          const std::string &keyLeft,
                          const std::string &keyRight) {
  auto startPos = content.find(keyLeft) + keyLeft.size();
  return content.substr(startPos, content.find(keyRight) - startPos);
}

int getNumber(const std::string &content) {
  std::regex pattern("-?\\d+");
  std::smatch match;

  if (std::regex_search(content, match, pattern)) {
    return std::stoi(match[0]);
  } else {
    // Handle case where no number is found
    Utils::Logging::logLine("ERROR: Couldn't extract number from string: ",
                            content);
    return 0; // Or throw an exception, return a default value, etc.
  }
}

}; // namespace Fwg::Parsing::Scenario
