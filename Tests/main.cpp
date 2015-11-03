#include <iostream>
#include <stdexcept>

#include "RPN/Parser.h"

#ifndef _MSC_VER
#define lest_FEATURE_COLOURISE 1
#endif

#include "lest.hpp"

using namespace std;

bool TestParse(const std::string &expr)
{
	auto p = RPN::Parser::Default().Parse(expr);
	return p != nullptr;
}

float TestValue(const std::string &expr)
{
	
	auto p = RPN::Parser::Default().Parse(expr);
	if (!p)
		throw std::runtime_error("Can't parse");
    auto c = RPN::Parser::Default().Compile(expr);
	if (!c)
		throw std::runtime_error("Can't compile");

	auto pv = p->value();
	auto cv = c();
	if (pv != cv)
		throw std::runtime_error("Compiled & interpreted result doesn't match");

	c.Release();
	return cv;
}

std::string TestValueString(const std::string &expr)
{
	auto p = RPN::Parser::Default().Parse(expr);
	if (!p)
		throw std::runtime_error("Can't parse");
	return p->stringValue();
}


const lest::test specification[] =
{
	CASE("Simplest Parse")
	{
		EXPECT(TestParse("2+2"));
		EXPECT(TestParse("2+2*3"));
		EXPECT(TestParse("2+2*+3") == false);
		EXPECT(TestParse("2 22") == false);
	},
	CASE("Parenthesis Parse")
	{
		EXPECT(TestParse("(1+1)"));
		EXPECT(TestParse("(2+2) * (2+3)"));
	},

	CASE("Simplest Value")
	{
		EXPECT(TestValue("2+2") == 4.0f);
		EXPECT(TestValue("2+2*3") == 8.0f);
	},
	CASE("Binary operators")
	{
		EXPECT(TestValue("2+2") == 4.0f);
		EXPECT(TestValue("2-2") == 0.0f);
		EXPECT(TestValue("2*3") == 6.0f);
		EXPECT(TestValue("6/3") == 2.0f);
	},

	CASE("Comparision operators")
	{
		EXPECT(TestValue("2<3") == 1.0f);
		EXPECT(TestValue("3<2") == 0.0f);
		EXPECT(TestValue("2<2") == 0.0f);

		EXPECT(TestValue("2<=3") == 1.0f);
		EXPECT(TestValue("3<=2") == 0.0f);
		EXPECT(TestValue("2<=2") == 1.0f);

		EXPECT(TestValue("3>2") == 1.0f);
		EXPECT(TestValue("2>3") == 0.0f);
		EXPECT(TestValue("2>2") == 0.0f);

		EXPECT(TestValue("3>=2") == 1.0f);
		EXPECT(TestValue("2>=3") == 0.0f);
		EXPECT(TestValue("2>=2") == 1.0f);
	},

	CASE("Equality operators")
	{
		EXPECT(TestValue("2==2") == 1.0f);
		EXPECT(TestValue("3==2") == 0.0f);

		EXPECT(TestValue("2!=2") == 0.0f);
		EXPECT(TestValue("3!=2") == 1.0f);
	},

	CASE("Boolean operators")
	{
		EXPECT(TestValue("2 && 2") == 1.0f);
		EXPECT(TestValue("0 && 2") == 0.0f);
		EXPECT(TestValue("0 && 0") == 0.0f);
		EXPECT(TestValue("(1 && 1) && (1 && 1)") == 1.0f);

		EXPECT(TestValue("2 || 2") == 1.0f);
		EXPECT(TestValue("0 || 2") == 1.0f);
		EXPECT(TestValue("0 || 0") == 0.0f);
		EXPECT(TestValue("(1 || 0) || (1 || 1)") == 1.0f);

		EXPECT(TestValue("(1 || 0) && (1 || 1)") == 1.0f);
		EXPECT(TestValue("(1 && 0) || (1 && 1)") == 1.0f);
	},

	CASE("Functions")
	{
		EXPECT(TestValue("math.min(-1,1) + 3") == 2.0f);
		EXPECT(TestValue("math.max(-1,1) + 3") == 4.0f);

		EXPECT(TestValue("if(1,2,3) + 2") == 4.0f);
		EXPECT(TestValue("if(0,2,3) + 2") == 5.0f);

		EXPECT(TestValue("math.min(math.max(-1,1),6) + 3") == 4.0f);
	},

	CASE("Functors/Functions with string args")
	{
		EXPECT(TestValue("string.length('Test')") == 4.0f);

		EXPECT(TestValue("stack.push(3)") == 3.0f);
		EXPECT(TestValue("stack.pop()") == 3.0f);

		EXPECT(TestValue("string.length('Test') + string.length('Test2')") == 9.0f);


		EXPECT(TestValue("stack.push(1) + stack.push(2)") == 3.0f);
		EXPECT(TestValue("stack.pop() + stack.pop()") == 3.0f);
	},

	CASE("String Functors")
	{
		EXPECT(TestValueString("string.join('aaa','bbb')") == "aaabbb");
		EXPECT(TestValueString("string.join('aaa',2)") == "aaa2");
		EXPECT(TestValueString("string.join(12,34)") == "1234");

		EXPECT(TestValue("string.length(string.join(1,2))") == 2.0f);
		EXPECT(TestValue("string.length(string.join(string.join('aaa','bbb'),1))") == 7.0f);
		//EXPECT(TestValueString("string.join(1,2,3)") == "123"); join isn't variadic right now
		//
	},
};

int main (int argc, char * argv[])
{
	return lest::run(specification, argc, argv);
}
