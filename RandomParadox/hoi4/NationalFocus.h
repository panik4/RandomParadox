#pragma once
#include <string>
#include <vector>
class NationalFocus
{
public:
	enum FocusType { attack, defense, military, politics, country };
	//define FocusType;
	FocusType fType;
	NationalFocus();
	NationalFocus(int ID, FocusType fType, bool default, std::string source, std::string dest, std::vector<int> date);
	~NationalFocus();

	int ID;
	bool default;
	std::string sourceTag;
	std::string destTag;
	std::vector<int> date;
	std::vector<int> position;
	int validity;
	double durationFactor = 1.0;
	std::vector<int> precedingFoci;

	bool atWar;
	int maxWarCount = 1;

	std::vector<std::string> conditionals;
	std::vector<std::string> effects; // e.g. set_country_flag
	std::string bypass;

	// organise positions
	static void organiseFoci(std::vector<NationalFocus>& foci);
};

