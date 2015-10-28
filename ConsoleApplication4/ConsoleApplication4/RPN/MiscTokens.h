#ifndef MXRPNMISCTOKENS
#define MXRPNMISCTOKENS
#include "Token.h"

namespace RPN
{

	class LeftParenthesis : public Token
	{
	public:
		Type type() override { return Type::LeftParenthesis; }
	};

	class RightParenthesis : public Token
	{
	public:
		Type type() override { return Type::RightParenthesis; }
	};

	class Comma : public Token
	{
	public:
		Type type() override { return Type::FunctionArgumentSeparator; }
	};

}

#endif