#include "NationalFocus.h"
int NationalFocus::IDcounter = 0;
NationalFocus::NationalFocus()
{
}

NationalFocus::NationalFocus(FocusType fType, bool default, std::string source, std::string dest, std::vector<int> date) : ID{ ID }, fType{ fType }, default{ default }, sourceTag{ source }, destTag{ dest }, date{ date }
{
	this->ID = IDcounter++;
	position = { 5,5 };
}

NationalFocus::~NationalFocus()
{
}

void NationalFocus::makeAlternative(std::vector<NationalFocus>& foci)
{
	for (auto& focus : foci)
	{
		for (auto& alternative : foci)
		{
			if (focus == alternative)
				continue;
			focus.alternativeFoci.push_back(alternative.ID);
		}
	}
}

std::ostream & operator<<(std::ostream & os, const NationalFocus & focus)
{
	auto action = "";
	switch (focus.fType)
	{
	case NationalFocus::FocusType::attack:
	{
		action = " attacks ";
		break;
	}
	case NationalFocus::FocusType::ally:
	{
		action = " tries to ally ";
		break;
	}
	case NationalFocus::FocusType::defense:
	{
		action = " defends ";
		break;

	}
	}
	os << focus.sourceTag << action << focus.destTag << std::endl;
	os << "Alternatives: " << focus.alternativeFoci.size() << "; Preceding Foci: " << focus.precedingFoci.size() << std::endl;
	return os;
}
