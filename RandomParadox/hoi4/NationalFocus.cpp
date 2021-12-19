#include "NationalFocus.h"

NationalFocus::NationalFocus()
{
}

NationalFocus::NationalFocus(int ID, FocusType fType, bool default, std::string source, std::string dest, std::vector<int> date) : ID{ID}, fType{ fType }, default{ default }, sourceTag{ source }, destTag{ dest }, date{ date }
{
	position = { 5,5 };
}

NationalFocus::~NationalFocus()
{
}


