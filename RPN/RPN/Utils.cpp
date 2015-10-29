#include "Utils.h"


namespace RPN
{
	bool eat_string_in_stream_if_equal(std::stringstream &ss, const std::string str)
	{
		auto position = ss.tellg();

		auto length = str.length();
		std::unique_ptr<char[]> buffer(new char[length + 1]);
		buffer[length] = 0;
		ss.read(buffer.get(), length);

		if (str == buffer.get())
			return true;

		ss.seekg(position);

		return false;
	}

	std::string eat_string_in_stream(std::stringstream &ss, const std::function<bool(char c, int index)>& isAllowed)
	{
		auto position = ss.tellg();

		int index = 0;
		while (true)
		{
			auto next_char = ss.peek();
			if (next_char == std::stringstream::traits_type::eof())
			{
				ss.clear();
				break;
			}
			bool allowed = isAllowed(next_char, index);
			if (!allowed)
				break;
			ss.get();
			index++;
		}
		auto position_end = ss.tellg();

		auto length = index;

		if (length == 0)
			return "";

		ss.seekg(position);
		std::unique_ptr<char[]> buffer(new char[length + 1]);
		buffer[length] = 0;
		ss.read(buffer.get(), length);

		return std::string(buffer.get(), length);
	}

	void setXmmVariable(asmjit::X86Compiler &c, asmjit::XmmVar &v, float d) {
		asmjit::X86GpVar temp = c.newGpVar();
		uint32_t *dd = (uint32_t *)(&d);
		c.mov(temp, *dd);
		c.movd(v, temp.m());
		c.unuse(temp);
	}

}

