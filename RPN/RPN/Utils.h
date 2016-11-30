#ifndef MXRPNUTILS
#define MXRPNUTILS

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#ifdef MX_JIT
#include <asmjit/asmjit.h>
#endif

namespace RPN
{
	bool eat_string_in_stream_if_equal(std::stringstream &ss, const std::string str);

	std::string eat_string_in_stream(std::stringstream &ss, const std::function<bool(char c, int index)>& isAllowed);

#ifdef MX_JIT
	void setXmmVariable(asmjit::X86Compiler &c, asmjit::XmmVar &v, float d);
#endif
}

#endif