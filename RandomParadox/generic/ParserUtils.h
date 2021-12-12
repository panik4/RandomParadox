#pragma once
#include <string>
#include <experimental/filesystem>
#include "../FastWorldGen/FastWorldGen/FastWorldGenerator.h"

class ParserUtils
{
	std::string hoi4Path;
	bool genHoi4;
public:
	static void writeFile(std::string path, std::string content)
	{
		ofstream myfile;
		myfile.open(path);
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

	static vector<std::string> readFilesInDirectory(std::string path)
	{
		const std::experimental::filesystem::path directory{ path };
		vector<std::string> fileContents;
		for (auto const& dir_entry : std::experimental::filesystem::directory_iterator{ directory })
		{
			std::stringstream pathStream;
			pathStream << dir_entry.path();
			std::string pathString;
			pathString = pathStream.str();
			//std::cout << pathString << '\n';
			//std::string filename = pathString.substr(pathString.find_last_of("\\") + 1, pathString.back() - pathString.find_last_of("\\"));
			//std::cout << filename << '\n';
			fileContents.push_back(readFile(pathString));
		}
		return fileContents;
	};

	static vector<std::string> getLines(std::string path)
	{
		vector<std::string> content;
		std::string line;
		ifstream myfile;
		myfile.open(path);
		while (getline(myfile, line))
		{
			content.push_back(line);
		}
		myfile.close();
		return content;
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
	static void replaceOccurences(std::string& content, std::string key, std::string value)
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
		auto pos = 0;
		pos = content.find(key);
		if (pos != string::npos)
		{
			auto blockEnd = content.find("}", pos) + 1; // find closing bracket
			return content.substr(pos, blockEnd - pos);
		}
		return "";
	};

	static void removeCharacter(std::string& content, char character)
	{
		content.erase(std::remove(content.begin(), content.end(), character), content.end());
	};

	static vector<std::string> getTokens(std::string& content, char delimiter)
	{
		vector<std::string> tokens{};

		stringstream sstream(content);
		string token;
		while (std::getline(sstream, token, delimiter))
			tokens.push_back(token);
		return tokens;
	};

	static vector<int> getNumbers(std::string& content, char delimiter, set<int> tokensToConvert)
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
	static vector<int> getNumberBlock(std::string content, std::string key)
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
};


