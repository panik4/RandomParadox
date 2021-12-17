#include "NameGenerator.h"



NameGenerator::NameGenerator()
{
	//tokenTemplates = ParserUtils::getLines("resources\\names\\token_templates.txt");
	nameRules = ParserUtils::getLines("resources\\names\\name_rules.txt");
	getMap("resources\\names\\token_groups.txt", groups);
	getMap("resources\\names\\state_types.txt", ideologyNames);
}


NameGenerator::~NameGenerator()
{
}



std::string NameGenerator::generateName()
{
	auto selectedRule = nameRules[Data::getInstance().random2() % nameRules.size()];// *select_random(nameRules);
	auto selectedRuleNum = ParserUtils::getTokens(selectedRule, ';');
	std::string name;
	name = getToken(selectedRuleNum); 
	//toupper(name.begin());
	std::transform(name.begin(), name.begin()+1, name.begin(), ::toupper);
	return name;
}

std::string NameGenerator::generateAdjective(std::string & name)
{
	for (auto vowel : groups["vowels"])
		if (vowel.front() == name.back())
			return name + getRandomMapElement("adjModifierVowel", groups);
	return name + getRandomMapElement("adjModifierConsonant", groups);
}

std::string NameGenerator::modifyWithIdeology(std::string & ideology, std::string name, std::string adjective)
{
	auto stateName = getRandomMapElement(ideology, ideologyNames);
	if (stateName.find("templateAdj") != std::string::npos)
	{
		ParserUtils::replaceOccurences(stateName, "templateAdj", adjective);
	}
	else {
		ParserUtils::replaceOccurences(stateName, "template", name);
	}
	return stateName;
}

std::string NameGenerator::generateTag(std::string name, set<std::string>& tags)
{
	std::string tag = "";
	int retries = 0;
	while ((tag.size() == 0 || tags.find(tag) != tags.end()) && retries++ < 10)
	{
		int offset = clamp(retries - 1, 0, (int)name.size() - 3);
		tag = name.substr(0 + offset, 3);
		std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
	}
	if (retries == 11)
	{
		tag = getRandomMapElement("consonants", groups) + getRandomMapElement("consonants", groups) + getRandomMapElement("consonants", groups);
		std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
	}
	tags.insert(tag);
	return tag;
}

void NameGenerator::getMap(std::string path, std::map<std::string, std::vector<std::string>>& map)
{
	auto groupLines = ParserUtils::getLines(path);
	for (auto& line : groupLines)
	{
		auto tokens = ParserUtils::getTokens(line, ';');
		for (int i = 1; i < tokens.size(); i++)
		{
			map[tokens[0]].push_back(tokens[i]);
		}
	}
}

std::string NameGenerator::getRandomMapElement(std::string key, std::map<std::string, std::vector<std::string>> map)
{
	return *select_random(map[key]);
}

std::string NameGenerator::getToken(vector<std::string>& rule)
{
	//auto tokenTemplate = rule[index];
	std::string retString = "";
	for (int i = 0; i < rule.size(); i++)
	{
		if (rule[i] == "c")
			retString += getRandomMapElement("consonants", groups);
		else if (rule[i] == "rc")
			retString += getRandomMapElement("rareConsonants", groups);
		else if (rule[i] == "gs")
			retString += getRandomMapElement("groupStart", groups);
		else if (rule[i] == "gm")
			retString += getRandomMapElement("groupMiddle", groups);
		else if (rule[i] == "ge")
			retString += getRandomMapElement("groupEnd", groups);
		else if (rule[i] == "gg")
			retString += getRandomMapElement("groupGeneric", groups);
		else {
			retString += getRandomMapElement("vowels", groups);
		}
	}
	return retString;
}
