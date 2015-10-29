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
		bool compilable() override { return _token ? _token->compilable() : false; }
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

		asmjit::X86XmmVar Compile(asmjit::X86Compiler& c) override
		{
			using namespace asmjit;
			auto token = _token->Compile(c);

			X86XmmVar minus(c);
			setXmmVariable(c, minus, -1.0f);
			c.mulss(token, minus);
			return token;
		}
	};
    

	class BinaryOperator : public Operator
	{
	public:
	
		int precedence() override { return 2; }

		bool constant() override { return (_tokens[0] && !_tokens[0]->constant()) || (_tokens[1] && !_tokens[1]->constant()) ? false : true; }

		bool compilable() override 
		{
			for (int i = 0; i < 2; i++)
			if (!_tokens[i] || !_tokens[i]->compilable())
				return false;
			return true; 
		}
	protected:
		void Parse(ParserContext &tokens) override
		{
			_tokens[1] = tokens.popAndParseToken();
			_tokens[0] = tokens.popAndParseToken();
		}
		
		asmjit::X86XmmVar Compile(asmjit::X86Compiler& c) override
		{
			using namespace asmjit;
			auto token1 = _tokens[0]->Compile(c);
			auto token2 = _tokens[1]->Compile(c);

			return BinaryCompile(c, token1, token2);
		}

		virtual asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2)
		{
			return o1;
		}


		float value_of(unsigned index) { return _tokens[index]->value(); }

		TokenPtr _tokens[2];
	};


	class BinaryPlusOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) + value_of(1); }

		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.addss(o1, o2);
			return o1;
		}
	};

	class BinaryMinusOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) - value_of(1); }

		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.subss(o1, o2);
			return o1;
		}
	};

	class BinaryMultiplyOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) * value_of(1); }
		int precedence() override { return 3; }

		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.mulss(o1, o2);
			return o1;
		}
	};

	class BinaryDivisionOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) / value_of(1); }
		int precedence() override { return 3; }

		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.divss(o1, o2);
			return o1;
		}
	};


	class BinaryLesserThanOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) < value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
		bool compilable() override { return false; }
	};

	class BinaryGreaterThanOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) > value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
		bool compilable() override { return false; }
	};


	class BinaryLesserOrEqualsOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) <= value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
		bool compilable() override { return false; }
	};

	class BinaryGreaterOrEqualsOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) >= value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
		bool compilable() override { return false; }
	};

	class BinaryEqualsOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) == value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 9; }
		bool compilable() override { return false; }
	};


	class BinaryAndOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) && value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 13; }
		bool compilable() override { return false; }
	};

	class BinaryOrOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) || value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 14; }
		bool compilable() override { return false; }
	};


}

#endif