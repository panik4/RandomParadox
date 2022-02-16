#pragma once
#include <string>
#include <vector>
#include "ParserUtils.h"
class NameGenerator
{
	// containers
	std::vector<std::string> nameRules;
	std::map<std::string, std::vector<std::string>> groups;
	std::map<std::string, std::vector<std::string>> ideologyNames;
public:
	NameGenerator();
	~NameGenerator();
	// member functions
	std::string generateName();
	std::string generateAdjective(std::string& name);
	std::string generateTag(std::string name, set<std::string>& tags);
	std::string getRandomMapElement(std::string key, std::map<std::string, std::vector<std::string>> map);
	std::string getToken(std::vector<std::string>& rule);
	std::string modifyWithIdeology(const std::string& ideology, std::string name, std::string adjective);
	void readMap(std::string path, std::map<std::string, std::vector<std::string>>& map);
};

