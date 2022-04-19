#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <map>
class NationalFocus
{
	static int IDcounter;
public:
	// typedefs
	enum FocusType { attack, ally, foundFaction, factionInvite, military, defense, politics, country };
	// constructors/destructors
	NationalFocus();
	NationalFocus(FocusType fType, bool defaultV, std::string source, std::string dest, std::vector<int> date);
	~NationalFocus();
	// member variables
	int ID;
	int stepID;
	int chainID;
	// define FocusType;
	FocusType fType;
	std::string sourceTag;
	std::string destTag;
	std::string bypass;
	bool defaultV;
	int validity;
	double durationFactor = 1.0;
	bool atWar;
	int maxWarCount = 1;

	// containers
	std::vector<int> date;
	std::vector<int> position;
	std::vector<int> precedingFoci;
	std::vector<int> andFoci;
	std::vector<int> alternativeFoci;
	std::vector<std::string> conditionals;
	std::vector<std::string> effects; // e.g. set_country_flag
	static std::map<std::string, NationalFocus::FocusType> typeMapping;

	// organise positions
	static void mapTypes();
	// operators
	friend std::ostream& operator<<(std::ostream& os, const NationalFocus& focus);
	bool operator==(const NationalFocus& right) const {
		return ID == right.ID;
	}
};

