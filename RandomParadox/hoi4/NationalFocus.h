#pragma once
#include <string>
#include <vector>
#include <iostream>
class NationalFocus
{
	static int IDcounter;
public:
	enum FocusType { attack, defense, ally, military, politics, country };
	//define FocusType;
	FocusType fType;
	NationalFocus();
	NationalFocus(FocusType fType, bool default, std::string source, std::string dest, std::vector<int> date);
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
	std::vector<int> alternativeFoci;

	bool atWar;
	int maxWarCount = 1;

	std::vector<std::string> conditionals;
	std::vector<std::string> effects; // e.g. set_country_flag
	std::string bypass;

	bool operator==(const NationalFocus& right) const
	{
		return ID == right.ID;
	}

	// organise positions
	static void organiseFoci(std::vector<NationalFocus>& foci);
	static void makeAlternative(std::vector<NationalFocus>& foci);
	friend std::ostream& operator<<(std::ostream& os, const NationalFocus& focus);
};

