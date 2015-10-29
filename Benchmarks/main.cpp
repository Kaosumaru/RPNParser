#include <iostream>
#include "RPN/Parser.h"


float TestParse(const std::string &expr)
{
	auto p = RPN::Parser::Default().Parse(expr);
	if (!p)
		return -666.0f;
	return p->value();
}

int main ()
{
  using namespace std;

  auto x = TestParse("2+2");

  cout << "Hello world" << endl;
  return 0;
}
