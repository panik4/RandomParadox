#pragma once
#include "FastWorldGenerator.h"
#include <filesystem>
#include <string>

namespace Scenario::ParserUtils {

void writeFile(const std::string &path, std::string content, bool utf8 = false);
std::string readFile(std::string path);

std::vector<std::string> readFilesInDirectory(const std::string &path);
std::vector<std::string> getLines(const std::string &path);
std::vector<std::vector<std::string>> getLinesByID(const std::string &path);
std::string csvFormat(const std::vector<std::string> arguments, char delimiter,
                      bool trailing);
void removeCharacter(std::string &content, char character);
std::vector<std::string> getTokens(const std::string &content,
                                   const char delimiter);

std::vector<int> getNumbers(const std::string &content, const char delimiter,
                            const std::set<int> tokensToConvert = {});
std::vector<int> getNumberBlock(std::string content, std::string key);

std::string replaceOccurences(std::string &content, std::string key,
                              std::string value);
// returns the line from a delimiter after a key to the end of the line
// helpful when the delimiter isn't always in the same format behind the key
// e.g. key=value and key =value
std::string getLineValue(const std::string &content, std::string key,
                         std::string delimiter);
// replace complete line from beginning of key to linebreak with value
void replaceLine(std::string &content, std::string key, std::string value);
// find the closing bracket of a block. Handles opening brackets correctly
// as long as every opening bracket has an opening bracket
size_t findClosingBracket(const std::string &content, size_t startPos);
// reads the bracket block including keyword onwards up until a closing
// bracket
std::string getBracketBlock(const std::string &content, const std::string key);
// reads the bracket block excluding keyword onwards up until a closing
// bracket
std::string getBracketBlockContent(const std::string &content,
                                   const std::string key);
// delete the bracket block from the bracket on, leaving the key
void removeBracketBlockFromBracket(std::string &content, const std::string key);
// delete the bracket block from the key on, leaving nothing
bool removeBracketBlockFromKey(std::string &content, const std::string key);

void removeSurroundingBracketBlock(std::string &content, const std::string key);
void removeSurroundingBracketBlockFromLineBreak(std::string &content,
                                                const std::string key);
}; // namespace Scenario::ParserUtils
