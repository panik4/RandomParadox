#pragma once
#include <string>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"

class ParserUtils
{
public:
	static void writeFile(std::string path, std::string content, bool utf8 = false)
	{
		std::ofstream myfile;
		myfile.open(path);
		if (utf8)
		{
			unsigned char bom[] = { 0xEF,0xBB,0xBF };
			myfile.write((char*)bom, sizeof(bom));
		}
		myfile << content;
		myfile.close();
	};
	static std::string readFile(std::string path)
	{
		std::string content;
		std::string line;
		std::ifstream myfile;
		myfile.open(path);
		while (getline(myfile, line))
		{
			content.append(line + "\n");
		}
		myfile.close();
		return content;
	};

	static std::vector<std::string> readFilesInDirectory(std::string path)
	{
		const std::experimental::filesystem::path directory{ path };
		std::vector<std::string> fileContents;
		for (auto const& dir_entry : std::experimental::filesystem::directory_iterator{ directory })
		{
			std::stringstream pathStream;
			pathStream << dir_entry.path();
			std::string pathString;
			pathString = pathStream.str();
			fileContents.push_back(readFile(pathString));
		}
		return fileContents;
	};

	static std::vector<std::string> getLines(std::string path)
	{
		std::vector<std::string> content;
		std::string line;
		std::ifstream myfile;
		myfile.open(path);
		while (getline(myfile, line))
		{
			if (line.front() == '#')
				continue;
			content.push_back(line);
		}
		myfile.close();
		return content;
	};

	static std::vector<std::vector<std::string>> getLinesByID(std::string path)
	{
		std::vector<std::vector<std::string>> sortedLines;
		sortedLines.resize(1000);
		int IDcounter = 0;
		std::string line;
		std::ifstream myfile;
		myfile.open(path);
		while (getline(myfile, line))
		{
			if (line.size() && line.front() != '#')
			{
				auto tokens = getTokens(line, ';');
				sortedLines[stoi(tokens[0])].push_back(line);
			}
		}
		myfile.close();
		return sortedLines;
	};

	static std::string csvFormat(std::vector<std::string> arguments, char delimiter, bool trailing)
	{
		std::vector<std::string>::iterator arg;
		std::string retString("");
		for (arg = arguments.begin(); arg != arguments.end(); arg++)
		{
			retString.append(*arg);
			if (!trailing && arguments.end() - arg == 1)
			{
				continue;
			}
			retString.append(std::string{ delimiter });

		}
		retString.append("\n");
		return retString;
	};
	static std::string replaceOccurences(std::string& content, std::string key, std::string value)
	{
		auto pos = 0;
		do
		{
			pos = content.find(key);
			if (pos != std::string::npos)
			{
				content.replace(pos, key.length(), value);
			}
		} while (pos != std::string::npos);
		return content;
	};

	static std::string getLineValue(std::string& content, std::string key, std::string value)
	{
		auto pos = 0;
		pos = content.find(key);
		if (pos != std::string::npos)
		{
			auto delimiterPos = content.find(value, pos) + 1;
			auto lineEnd = content.find("\n", pos);
			return content.substr(delimiterPos, lineEnd - delimiterPos);
		}
	};

	static void replaceLine(std::string& content, std::string key, std::string value)
	{
		auto pos = 0;
		pos = content.find(key);
		if (pos != std::string::npos)
		{
			auto lineEnd = content.find("\n", pos);
			content.replace(pos, lineEnd - pos, value);
		}
	};
	static int findClosingBracket(std::string& content, size_t startPos) {
		// find opening bracket of this block
		auto openingBracket = content.find("{", startPos) + 1;
		// find next opening bracket
		auto nextOpenBracket = content.find("{", openingBracket+1);
		// find closing bracket
		auto blockEnd = content.find("}", startPos) + 1;
		// found an opening bracket before the closing bracket, means this bracket doesn't close the scope we search
		while (nextOpenBracket != std::string::npos && nextOpenBracket < blockEnd) {
			blockEnd = content.find("}", blockEnd) + 1; // find the next closing bracket
			nextOpenBracket = content.find("{", nextOpenBracket + 1); // find next opening bracket
		}
		return blockEnd;
	}
	// reads the bracket block form a keyword onwards up until a closing bracket
	static std::string getBracketBlock(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		if (pos != std::string::npos) {
			auto blockEnd = findClosingBracket(content, pos);
			return content.substr(pos, blockEnd - pos);
		}
		return "";
	};
	static std::string getBracketBlockContent(std::string& content, std::string key)
	{
		// first get whole block of keyword
		auto block = getBracketBlock(content, key);
		// now get the opening bracket
		auto pos = block.find("{") + 1;
		if (pos != std::string::npos) {
			auto blockEnd = findClosingBracket(content, pos);
			return block.substr(pos, blockEnd - pos);
		}
		return "";
	};
	// delete the bracket block from the bracket on, leaving the key
	static std::string removeBracketBlockFromBracket(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		auto openingBracket = content.find("{", pos) + 1;
		if (pos != std::string::npos) {
			auto blockEnd = findClosingBracket(content, pos);
			return content.erase(openingBracket, blockEnd - openingBracket);
		}
		return "";
	};
	// delete the bracket block from the key on, leaving nothing
	static std::string removeBracketBlockFromKey(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		if (pos != std::string::npos) {
			auto blockEnd = findClosingBracket(content, pos);
			return content.erase(pos, blockEnd - pos);
		}
		return "";
	};
	static void removeCharacter(std::string& content, char character)
	{
		content.erase(std::remove(content.begin(), content.end(), character), content.end());
	};

	static std::vector<std::string> getTokens(std::string& content, char delimiter)
	{
		std::vector<std::string> tokens{};
		std::stringstream sstream(content);
		std::string token;
		while (std::getline(sstream, token, delimiter))
			tokens.push_back(token);
		return tokens;
	};

	static std::vector<int> getNumbers(std::string& content, char delimiter, std::set<int> tokensToConvert)
	{
		bool convertAll = false;
		if (!tokensToConvert.size())
			convertAll = true;
		std::vector<int> numbers{};
		std::stringstream sstream(content);
		std::string token;
		int counter = 0;
		while (std::getline(sstream, token, delimiter))
		{
			if (token.size())
				if (convertAll || tokensToConvert.find(counter) != tokensToConvert.end())
					numbers.push_back(stoi(token));
			counter++;
		}
		return numbers;
	};
	static std::vector<int> getNumberBlock(std::string content, std::string key)
	{
		auto bracketBlock = getBracketBlock(content, key);
		removeCharacter(bracketBlock, '{');
		removeCharacter(bracketBlock, '\n');
		removeCharacter(bracketBlock, '\t');
		removeCharacter(bracketBlock, '=');
		removeCharacter(bracketBlock, '}');
		replaceOccurences(bracketBlock, key, "");
		return getNumbers(bracketBlock, ' ', std::set<int>{});
	}
	template <class T>
	static std::string ostreamToString(T elem)
	{
		std::stringstream ss;
		ss << elem;
		return ss.str();
	}
	static std::string removeSurroundingBracketBlock(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		if (pos != std::string::npos) {
			pos = content.rfind("{", pos);
			auto blockEnd = findClosingBracket(content, pos);
			content.erase(pos, blockEnd - pos);
			return content;
		}
		return "";
	};
	static std::string removeSurroundingBracketBlockFromLineBreak(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		if (pos != std::string::npos) {
			pos = content.rfind("{", pos);
			pos = content.rfind("\n", pos);
			auto blockEnd = findClosingBracket(content, pos);
			content.erase(pos, blockEnd - pos);
			return content;
		}
		return "";
	};
};


