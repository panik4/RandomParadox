#pragma once
#include <string>
#include <vector>
#include "ParserUtils.h"
class NameGenerator
{
	vector<std::string> nameRules;
	//vector<std::string> tokenTemplates;
	std::map<std::string, std::vector<std::string>> groups;
	std::map<std::string, std::vector<std::string>> ideologyNames;
public:
	NameGenerator();
	~NameGenerator();
	std::string generateName();
	std::string generateAdjective(std::string& name);
	std::string modifyWithIdeology(std::string& ideology, std::string name, std::string adjective);
	std::string generateTag(std::string name, set<std::string>& tags);
	void getMap(std::string path, std::map<std::string, std::vector<std::string>>& map);
	std::string getRandomMapElement(std::string key, std::map<std::string, std::vector<std::string>> map);
	//std::string getVowel();
	//std::string getConsonant();
	//std::string getSpecial();
	std::string getToken(vector<std::string>& rule);
};

