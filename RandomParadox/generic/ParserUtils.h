#pragma once
#include <string>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"

class ParserUtils
{
public:
	static void writeFile(std::string path, std::string content, bool utf8=false)
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
		ifstream myfile;
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
		vector<std::string> fileContents;
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
		vector<std::string> content;
		std::string line;
		ifstream myfile;
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

	static std::vector<vector<std::string>> getLinesByID(std::string path)
	{
		vector<vector<std::string>> sortedLines;
		sortedLines.resize(1000);
		int IDcounter = 0;
		std::string line;
		ifstream myfile;
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

	static std::string csvFormat(vector<std::string> arguments, char delimiter, bool trailing)
	{
		vector<string>::iterator arg;
		std::string retString("");
		for (arg = arguments.begin(); arg != arguments.end(); arg++)
		{
			retString.append(*arg);
			if (!trailing && arguments.end() - arg == 1)
			{
				continue;
			}
			retString.append(string{ delimiter });

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
			if (pos != string::npos)
			{
				content.replace(pos, key.length(), value);
			}
		} while (pos != string::npos);
		return content;
	};

	static std::string getLineValue(std::string& content, std::string key, std::string value)
	{
		auto pos = 0;
		pos = content.find(key);
		if (pos != string::npos)
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
		if (pos != string::npos)
		{
			auto lineEnd = content.find("\n", pos);
			content.replace(pos, lineEnd - pos, value);
		}
	};

	static std::string getBracketBlock(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		if (pos != string::npos)
		{
			auto blockEnd = content.find("}", pos) + 1; // find closing bracket
			return content.substr(pos, blockEnd - pos);
		}
		return "";
	};
	static std::string getBracketBlockContent(std::string& content, std::string key)
	{
		auto block = getBracketBlock(content, key);
		auto pos = block.find("{") + 1;
		if (pos != string::npos)
		{
			auto blockEnd = block.find("}", pos); // find closing bracket
			return block.substr(pos, blockEnd - pos);
		}
		return "";
	};
	static void removeCharacter(std::string& content, char character)
	{
		content.erase(std::remove(content.begin(), content.end(), character), content.end());
	};

	static std::vector<std::string> getTokens(std::string& content, char delimiter)
	{
		vector<std::string> tokens{};

		stringstream sstream(content);
		string token;
		while (std::getline(sstream, token, delimiter))
			tokens.push_back(token);
		return tokens;
	};

	static std::vector<int> getNumbers(std::string& content, char delimiter, set<int> tokensToConvert)
	{
		bool convertAll = false;
		if (!tokensToConvert.size())
			convertAll = true;
		vector<int> numbers{};
		stringstream sstream(content);
		string token;
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
		return getNumbers(bracketBlock, ' ', set<int>{});
	}
	template <class T>
	static std::string ostreamToString(T elem)
	{
		std::stringstream ss;
		ss << elem;
		return ss.str();
	}
	static std::string removeBracketBlock(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		if (pos != string::npos) {
			pos = content.rfind("{", pos);
			auto blockEnd = content.find("}", pos) + 1; // find closing bracket
			content.erase(pos, blockEnd - pos);
			return content;
		}
		return "";
	};
	static std::string removeBracketBlockFromLineBreak(std::string& content, std::string key)
	{
		auto pos = content.find(key);
		if (pos != string::npos) {
			pos = content.rfind("{", pos);
			pos = content.rfind("\n", pos);
			auto blockEnd = content.find("}", pos) + 1; // find closing bracket
			content.erase(pos, blockEnd - pos);
			return content;
		}
		return "";
	};
};


