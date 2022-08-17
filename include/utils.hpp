#pragma once

#include <string>
#include <sstream>

template<class T>
std::string convert_string(const T& any)
{
	std::stringstream str;

	str << any;
	return str.str();
}
