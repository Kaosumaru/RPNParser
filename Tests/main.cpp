#include <iostream>
#include "RPN/Parser.h"

#ifndef _MSC_VER
#define lest_FEATURE_COLOURISE 1
#endif

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
	bool res = pv == expected && cv == expected;

	if (res)
		return true;
	return false;
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
	CASE("Parenthesis Parse")
	{
		EXPECT(TestParse("(1+1)", true));
		EXPECT(TestParse("(2+2) * (2+3)", true));
	},

	CASE("Simplest Value")
	{
		EXPECT(TestValue("2+2", 4.0f));
		EXPECT(TestValue("2+2*3", 8.0f));
	},
	CASE("Binary operators")
	{
		EXPECT(TestValue("2+2", 4.0f));
		EXPECT(TestValue("2-2", 0.0f));
		EXPECT(TestValue("2*3", 6.0f));
		EXPECT(TestValue("6/3", 2.0f));
	},

	CASE("Comparision operators")
	{
		EXPECT(TestValue("2<3", 1.0f));
		EXPECT(TestValue("3<2", 0.0f));
		EXPECT(TestValue("2<2", 0.0f));

		EXPECT(TestValue("2<=3", 1.0f));
		EXPECT(TestValue("3<=2", 0.0f));
		EXPECT(TestValue("2<=2", 1.0f));

		EXPECT(TestValue("3>2", 1.0f));
		EXPECT(TestValue("2>3", 0.0f));
		EXPECT(TestValue("2>2", 0.0f));

		EXPECT(TestValue("3>=2", 1.0f));
		EXPECT(TestValue("2>=3", 0.0f));
		EXPECT(TestValue("2>=2", 1.0f));
	},

	CASE("Equality operators")
	{
		EXPECT(TestValue("2==2", 1.0f));
		EXPECT(TestValue("3==2", 0.0f));

		EXPECT(TestValue("2!=2", 0.0f));
		EXPECT(TestValue("3!=2", 1.0f));
	},

	CASE("Boolean operators")
	{
		EXPECT(TestValue("2 && 2", 1.0f));
		EXPECT(TestValue("0 && 2", 0.0f));
		EXPECT(TestValue("0 && 0", 0.0f));
		EXPECT(TestValue("(1 && 1) && (1 && 1)", 1.0f));

		EXPECT(TestValue("2 || 2", 1.0f));
		EXPECT(TestValue("0 || 2", 1.0f));
		EXPECT(TestValue("0 || 0", 0.0f));
		EXPECT(TestValue("(1 || 0) || (1 || 1)", 1.0f));

		EXPECT(TestValue("(1 || 0) && (1 || 1)", 1.0f));
		EXPECT(TestValue("(1 && 0) || (1 && 1)", 1.0f));
	},

	CASE("Functions")
	{
		EXPECT(TestValue("math.min(-1,1) + 3", 2.0f));
		EXPECT(TestValue("math.max(-1,1) + 3", 4.0f));

		EXPECT(TestValue("if(1,2,3) + 2", 4.0f));
		EXPECT(TestValue("if(0,2,3) + 2", 5.0f));

		EXPECT(TestValue("math.min(math.max(-1,1),6) + 3", 4.0f));
	},
};

int main (int argc, char * argv[])
{
	return lest::run(specification, argc, argv);
}
