#pragma once
#include "FastWorldGenerator.h"
#include <filesystem>
#include <string>

class ParserUtils {

public:
  static void writeFile(const std::string &path, std::string content,
                        bool utf8 = false) {
    std::ofstream file;
    file.open(path);
    if (!file)
      throw std::exception(
          UtilLib::varsToString("Didn't manage to write to file ", path)
              .c_str());
    if (utf8) {
      const std::array<unsigned char, 3> bom[] = {0xEF, 0xBB, 0xBF};
      file.write((char *)bom, sizeof(bom));
    }
    file << content;
    file.close();
  };
  static std::string readFile(std::string path) {
    std::string content{""};
    std::string line;
    std::ifstream file;
    file.open(path);
    if (!file)
      throw std::exception(
          UtilLib::varsToString("Didn't manage to read from file ", path)
              .c_str());
    while (getline(file, line)) {
      content.append(line + "\n");
    }
    file.close();
    return content;
  };

  static std::vector<std::string>
  readFilesInDirectory(const std::string &path) {
    const std::filesystem::path directory{path};
    std::vector<std::string> fileContents;
    for (auto const &dir_entry :
         std::filesystem::directory_iterator{directory}) {
      std::stringstream pathStream{dir_entry.path().string()};
      std::string pathString{pathStream.str()};
      fileContents.push_back(readFile(pathString));
    }
    return fileContents;
  };

  static std::vector<std::string> getLines(const std::string &path) {
    std::vector<std::string> content;
    std::string line;
    std::stringstream file{readFile(path)};
    while (getline(file, line)) {
      if (line.front() == '#')
        continue;
      content.push_back(line);
    }
    return content;
  };

  static std::vector<std::vector<std::string>>
  getLinesByID(const std::string &path) {
    std::vector<std::vector<std::string>> sortedLines(10000);
    std::string line;
    std::stringstream file{readFile(path)};
    while (getline(file, line)) {
      if (line.size() && line.front() != '#') {
        auto tokens = getTokens(line, ';');
        if (tokens.size())
          sortedLines[stoi(tokens[0])].push_back(line);
      }
    }
    return sortedLines;
  };

  static std::string csvFormat(std::vector<std::string> arguments,
                               char delimiter, bool trailing) {
    std::vector<std::string>::iterator arg;
    std::string retString("");
    for (arg = arguments.begin(); arg != arguments.end(); arg++) {
      retString.append(*arg);
      if (!trailing && arguments.end() - arg == 1)
        continue;
      retString.append(std::string{delimiter});
    }
    retString.append("\n");
    return retString;
  };
  static void removeCharacter(std::string &content, char character) {
    content.erase(std::remove(content.begin(), content.end(), character),
                  content.end());
  };

  static std::vector<std::string> getTokens(const std::string &content,
                                            const char delimiter) {
    std::vector<std::string> tokens{};
    std::stringstream sstream(content);
    std::string token;
    while (std::getline(sstream, token, delimiter))
      tokens.push_back(token);
    return tokens;
  };

  static std::vector<int> getNumbers(const std::string &content,
                                     const char delimiter,
                                     const std::set<int> tokensToConvert = {}) {
    bool convertAll = !tokensToConvert.size();
    std::vector<int> numbers{};
    std::stringstream sstream(content);
    std::string token;
    int counter = 0;
    while (std::getline(sstream, token, delimiter)) {
      if (token.size())
        if (convertAll ||
            tokensToConvert.find(counter) != tokensToConvert.end())
          numbers.push_back(stoi(token));
      counter++;
    }
    return numbers;
  };
  static std::vector<int> getNumberBlock(std::string content, std::string key) {
    auto bracketBlock = getBracketBlock(content, key);
    removeCharacter(bracketBlock, '{');
    removeCharacter(bracketBlock, '\n');
    removeCharacter(bracketBlock, '\t');
    removeCharacter(bracketBlock, '=');
    removeCharacter(bracketBlock, '}');
    replaceOccurences(bracketBlock, key, "");
    return getNumbers(bracketBlock, ' ', std::set<int>{});
  }

  static std::string replaceOccurences(std::string &content, std::string key,
                                       std::string value) {
    size_t pos = 0;
    do {
      pos = content.find(key);
      if (pos != std::string::npos)
        content.replace(pos, key.length(), value);
    } while (pos != std::string::npos);
    return content;
  };
  // returns the line from a delimiter after a key to the end of the line
  // helpful when the delimiter isn't always in the same format behind the key
  // e.g. key=value and key =value
  static std::string getLineValue(const std::string &content, std::string key,
                                  std::string delimiter) {
    size_t pos = 0;
    pos = content.find(key);
    if (pos != std::string::npos) {
      const auto delimiterPos = content.find(delimiter, pos) + 1;
      const auto lineEnd = content.find("\n", pos);
      return content.substr(delimiterPos, lineEnd - delimiterPos);
    }
    return "";
  };
  // replace complete line from beginning of key to linebreak with value
  static void replaceLine(std::string &content, std::string key,
                          std::string value) {
    size_t pos = 0;
    pos = content.find(key);
    if (pos != std::string::npos) {
      const auto lineEnd = content.find("\n", pos);
      content.replace(pos, lineEnd - pos, value);
    }
  };
  // find the closing bracket of a block. Handles opening brackets correctly
  // as long as every opening bracket has an opening bracket
  static size_t findClosingBracket(const std::string &content,
                                   size_t startPos) {
    // find opening bracket of this block
    const auto openingBracket = content.find("{", startPos);
    // find next opening bracket
    auto nextOpenBracket = content.find("{", openingBracket + 1);
    // find closing bracket
    auto blockEnd = content.find("}", startPos);
    // found an opening bracket before the closing bracket, means this bracket
    // doesn't close the scope we search
    while (nextOpenBracket != std::string::npos && nextOpenBracket < blockEnd) {
      blockEnd =
          content.find("}", blockEnd + 1); // find the next closing bracket
      nextOpenBracket =
          content.find("{", nextOpenBracket + 1); // find next opening bracket
    }
    return blockEnd;
  }
  // reads the bracket block including keyword onwards up until a closing
  // bracket
  static std::string getBracketBlock(const std::string &content,
                                     const std::string key) {
    const auto pos = content.find(key);
    if (pos != std::string::npos) {
      const auto blockEnd = findClosingBracket(content, pos) + 1;
      return content.substr(pos, blockEnd - pos);
    }
    return "";
  };
  // reads the bracket block excluding keyword onwards up until a closing
  // bracket
  static std::string getBracketBlockContent(const std::string &content,
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
  static void removeBracketBlockFromBracket(std::string &content,
                                            const std::string key) {
    const auto pos = content.find(key);
    const auto openingBracket = content.find("{", pos);
    if (pos != std::string::npos) {
      const auto blockEnd = findClosingBracket(content, pos);
      content.erase(openingBracket, blockEnd - openingBracket + 1);
    }
  };
  // delete the bracket block from the key on, leaving nothing
  static void removeBracketBlockFromKey(std::string &content,
                                        const std::string key) {
    const auto pos = content.find(key);
    if (pos != std::string::npos) {
      const auto blockEnd = findClosingBracket(content, pos);
      content.erase(pos, blockEnd - pos + 1);
    }
  };

  static void removeSurroundingBracketBlock(std::string &content,
                                            const std::string key) {
    auto pos = content.find(key);
    if (pos != std::string::npos) {
      pos = content.rfind("{", pos);
      const auto blockEnd = findClosingBracket(content, pos);
      content.erase(pos, blockEnd - pos);
    }
  };
  static void
  removeSurroundingBracketBlockFromLineBreak(std::string &content,
                                             const std::string key) {
    auto pos = content.find(key);
    if (pos != std::string::npos) {
      pos = content.rfind("{", pos);
      pos = content.rfind("\n", pos);
      auto blockEnd = findClosingBracket(content, pos);
      content.erase(pos, blockEnd - pos + 1);
    }
  };
};
