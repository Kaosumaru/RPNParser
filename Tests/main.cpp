#include <iostream>
#include "RPN/Parser.h"
#include "lest.hpp"

using namespace std;

bool TestParse(const std::string &expr, bool expected)
{
	auto p = RPN::Parser::Default().Parse(expr);
	bool parsed = p != nullptr;
	return expected == parsed;
}

bool TestValue(const std::string &expr, float expected)
{
	auto p = RPN::Parser::Default().Parse(expr);
	if (!p)
		return false;
    auto c = RPN::Parser::Default().Compile(expr);
	if (!c)
		return false;

	auto pv = p->value();
	auto cv = c();
	return pv == expected && cv == expected;
}


const lest::test specification[] =
{
	CASE("Simplest Parse")
	{
		EXPECT(TestParse("2+2", true));
		EXPECT(TestParse("2+2*3", true));
		EXPECT(TestParse("2+2*+3", false));
		EXPECT(TestParse("2 22", false));
	},
	CASE("Simplest Value")
	{
		EXPECT(TestValue("2+2", 4.0f));
		EXPECT(TestValue("2+2*3", 8.0f));
	},
};

int main (int argc, char * argv[])
{
	return lest::run(specification, argc, argv);
}
