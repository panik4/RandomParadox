#include "NameGenerator.h"



NameGenerator::NameGenerator()
{
	tokenTemplates = ParserUtils::getLines("resources\\names\\token_templates.txt");
	nameRules = ParserUtils::getLines("resources\\names\\name_rules.txt");
}


NameGenerator::~NameGenerator()
{
}



std::string NameGenerator::generateName()
{
	auto selectedRule = nameRules[Data::getInstance().random2() % nameRules.size()];// *select_random(nameRules);
	auto selectedRuleNum = ParserUtils::getNumbers(selectedRule, ';', std::set<int>{});
	std::string name;
	for (auto token : selectedRuleNum)
	{
		name += getToken(token);
	}
	return name;
}

std::string NameGenerator::generateTag(std::string name, set<std::string>& tags)
{
	std::string tag = "";
	int retries = 0;
	while ((tag.size() == 0 || tags.find(tag) == tags.end()) && retries++ < 10)
	{
		tag = name.substr(0, 3);
		std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
	}
	if (retries == 10)
	{
		tag = getConsonant() + getConsonant() + getConsonant();
	}
	tags.insert(tag);
	return tag;
}



std::string NameGenerator::getVowel()
{
	std::vector<std::string> vowels{ "a", "e", "i", "o", "u", "y" };
	return *select_random(vowels);
}

std::string NameGenerator::getConsonant()
{
	std::vector<std::string> consonants{ "b", "c","d","f","g","h","j","k","l","m","n","p","q","r","s","t","v", "w", "x", "z" };
	return *select_random(consonants);
}

std::string NameGenerator::getSpecial()
{
	std::vector<std::string> specials{ "ch", "sch" };
	return  *select_random(specials);
}

std::string NameGenerator::getToken(int index)
{
	auto tokenTemplate = tokenTemplates[index];
	//std::cout << tokenTemplate << std::endl;
	std::string retString = "";
	for (int i = 0; i < tokenTemplate.size(); i++)
	{
		if (tokenTemplate[i] == 'c')
			retString += getConsonant();
		else if (tokenTemplate[i] == 'g')
			retString += getSpecial();
		else {
			retString += getVowel();
		}
	}
	return retString;
}
