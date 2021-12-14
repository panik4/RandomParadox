#pragma once
#include <string>
#include <vector>
#include "ParserUtils.h"
class NameGenerator
{
	vector<std::string> nameRules;
	vector<std::string> tokenTemplates;
public:
	NameGenerator();
	~NameGenerator();
	std::string generateName();
	std::string generateTag(std::string name, set<std::string>& tags);
	std::string getVowel();
	std::string getConsonant();
	std::string getSpecial();
	std::string getToken(int index);
};

