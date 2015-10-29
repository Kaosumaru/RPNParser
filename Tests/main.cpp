#include <iostream>
#include "RPN/Parser.h"
#include "lest.hpp"

using namespace std;

float TestParse(const std::string &expr)
{
	auto p = RPN::Parser::Default().Parse(expr);
	if (!p)
		return -666.0f;
	return p->value();
}


const lest::test specification[] =
{
	CASE("Simplest parse test")
	{
		EXPECT(4 == TestParse("2+2"));
		EXPECT(8 == TestParse("2+2*3"));
	},

};

int main (int argc, char * argv[])
{
	return lest::run(specification, argc, argv);
}
