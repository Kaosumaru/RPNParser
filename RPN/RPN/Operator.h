#ifndef MXRPNOPERATOR
#define MXRPNOPERATOR
#include "Token.h"

namespace RPN
{

	class Operator : public Token
	{
	public:

		Type type() override { return Type::Operator; }

	};

    
	class UnaryOperator : public Operator
	{
	public:
        bool left_associative() override { return false; }
		int precedence() override { return 10; }
		bool constant() override { return _token ? _token->constant() : true; }
	protected:
		void Parse(ParserContext &tokens) override
		{
			_token = tokens.popAndParseToken();
		}
		
        
		float token_value() { return _token->value(); }
        
		TokenPtr _token;
	};
    
    class UnaryMinusOperator : public UnaryOperator
	{
	public:
		float value() override { return -token_value(); }
	};
    

	class BinaryOperator : public Operator
	{
	public:
	
		int precedence() override { return 2; }

		bool constant() override { return _tokens[0] && !_tokens[0]->constant() || _tokens[1] && !_tokens[1]->constant() ? false : true; }
	protected:
		void Parse(ParserContext &tokens) override
		{
			_tokens[1] = tokens.popAndParseToken();
			_tokens[0] = tokens.popAndParseToken();

		}
		

		float value_of(unsigned index) { return _tokens[index]->value(); }

		TokenPtr _tokens[2];
	};


	class BinaryPlusOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) + value_of(1); }
	};

	class BinaryMinusOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) - value_of(1); }
	};

	class BinaryMultiplyOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) * value_of(1); }
		int precedence() override { return 3; }
	};

	class BinaryDivisionOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) / value_of(1); }
		int precedence() override { return 3; }
	};


	class BinaryLesserThanOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) < value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};

	class BinaryGreaterThanOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) > value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};


	class BinaryLesserOrEqualsOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) <= value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};

	class BinaryGreaterOrEqualsOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) >= value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};

	class BinaryEqualsOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) == value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 9; }
	};


	class BinaryAndOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) && value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 13; }
	};

	class BinaryOrOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) || value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 14; }
	};


}

#endif