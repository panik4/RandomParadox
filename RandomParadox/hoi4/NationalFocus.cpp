#include "NationalFocus.h"

NationalFocus::NationalFocus()
{
}

NationalFocus::NationalFocus(int ID, FocusType fType, bool default, std::string source, std::string dest, std::vector<int> date) : ID{ ID }, fType{ fType }, default{ default }, sourceTag{ source }, destTag{ dest }, date{ date }
{
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


