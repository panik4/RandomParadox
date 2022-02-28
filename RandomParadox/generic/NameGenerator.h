#pragma once
#include <string>
#include <vector>
#include "ParserUtils.h"
class NameGenerator
{
	// containers
	static std::vector<std::string> nameRules;
	static std::map<std::string, std::vector<std::string>> groups;
	static std::map<std::string, std::vector<std::string>> ideologyNames;
public:
	// member functions
	static std::string generateName();
	static std::string generateAdjective(std::string& name);
	static std::string generateTag(std::string name, std::set<std::string>& tags);
	static std::string getRandomMapElement(std::string key, std::map<std::string, std::vector<std::string>> map);
	static std::string getToken(std::vector<std::string>& rule);
	static std::string modifyWithIdeology(const std::string& ideology, std::string name, std::string adjective);
	static void readMap(std::string path, std::map<std::string, std::vector<std::string>>& map);
	static void prepare();
};

