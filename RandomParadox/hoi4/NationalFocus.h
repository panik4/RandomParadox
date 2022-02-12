#pragma once
#include <string>
#include <vector>
#include <iostream>
class NationalFocus
{
	static int IDcounter;
public:
	// typedefs
	enum FocusType { attack, defense, ally, military, politics, country };
	// constructors/destructors
	NationalFocus();
	NationalFocus(FocusType fType, bool default, std::string source, std::string dest, std::vector<int> date);
	~NationalFocus();
	// member variables
	int ID;
	// define FocusType;
	FocusType fType;
	std::string sourceTag;
	std::string destTag;
	std::string bypass;
	bool default;
	int validity;
	double durationFactor = 1.0;
	bool atWar;
	int maxWarCount = 1;

	// containers
	std::vector<int> date;
	std::vector<int> position;
	std::vector<int> precedingFoci;
	std::vector<int> alternativeFoci;
	std::vector<std::string> conditionals;
	std::vector<std::string> effects; // e.g. set_country_flag

	// organise positions
	static void organiseFoci(std::vector<NationalFocus>& foci);
	static void makeAlternative(std::vector<NationalFocus>& foci);
	// operators
	friend std::ostream& operator<<(std::ostream& os, const NationalFocus& focus);
	bool operator==(const NationalFocus& right) const {
		return ID == right.ID;
	}
};

