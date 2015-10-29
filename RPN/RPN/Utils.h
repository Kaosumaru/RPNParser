#ifndef MXRPNUTILS
#define MXRPNUTILS

#include <functional>
#include <map>
#include <memory>
#include <sstream>


namespace RPN
{
	bool eat_string_in_stream_if_equal(std::stringstream &ss, const std::string str);

	std::string eat_string_in_stream(std::stringstream &ss, const std::function<bool(char c, int index)>& isAllowed);
}

#endif