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


	class ComparisionOperator : public BinaryOperator
	{
	protected:
		enum class Operator
		{
			Equal = 0,
			LessThan,
			LessOrEqual,
			Unordered,
			NotEqual,
			NotLessThan,
			NotLessOrEqual,
			Ordered,
		};

		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			asmjit::X86XmmVar out(c, asmjit::kX86VarTypeXmmSs);
			setXmmVariable(c, out, 1.0f);
			c.cmpss(o1, o2, (int)_imm);
			c.andps(out, o1);
			return out;
		}

		Operator _imm = Operator::Equal;
	};

	class BinaryLesserThanOperator : public ComparisionOperator
	{
	public:
		BinaryLesserThanOperator() { _imm = Operator::LessThan; }

		float value() override { return (value_of(0) < value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};

	class BinaryGreaterThanOperator : public ComparisionOperator
	{
	public:
		BinaryGreaterThanOperator() { _imm = Operator::NotLessOrEqual; }

		float value() override { return (value_of(0) > value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};


	class BinaryLesserOrEqualsOperator : public ComparisionOperator
	{
	public:
		BinaryLesserOrEqualsOperator() { _imm = Operator::LessOrEqual; }

		float value() override { return (value_of(0) <= value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};

	class BinaryGreaterOrEqualsOperator : public ComparisionOperator
	{
	public:
		BinaryGreaterOrEqualsOperator() { _imm = Operator::NotLessThan; }

		float value() override { return (value_of(0) >= value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 8; }
	};

	class BinaryEqualsOperator : public ComparisionOperator
	{
	public:
		BinaryEqualsOperator() { _imm = Operator::Equal; }

		float value() override { return (value_of(0) == value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 9; }
	};

	class BinaryNotEqualsOperator : public ComparisionOperator
	{
	public:
		BinaryNotEqualsOperator() { _imm = Operator::NotEqual; }

		float value() override { return (value_of(0) != value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 9; }
	};


	class BinaryAndOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) && value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 13; }
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.andps(o1, o2);

			asmjit::X86XmmVar zero(c, asmjit::kX86VarTypeXmmSs);
			setXmmVariable(c, zero, 0.0f);
			c.cmpss(o1, zero, 4); //o1 != zero

			setXmmVariable(c, zero, 1.0f); //well, abuse zero
			c.andps(o1, zero);
			return o1;
		}
	};

	class BinaryOrOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) || value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 14; }
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.orps(o1, o2);

			asmjit::X86XmmVar zero(c, asmjit::kX86VarTypeXmmSs);
			setXmmVariable(c, zero, 0.0f);
			c.cmpss(o1, zero, 4); //o1 != zero

			setXmmVariable(c, zero, 1.0f); //well, abuse zero
			c.andps(o1, zero);
			return o1;
		}
	};


}

#endif