#include <iostream>
#include "RPN/Parser.h"


void TestParse(const std::string &expr)
{
	//auto f = RPN::Parser::Default().Compile(expr);
	auto p = RPN::Parser::Default().Parse(expr);
}

int main ()
{
  using namespace std;

  TestParse("2+2");

  cout << "Hello world" << endl;
  return 0;
}
