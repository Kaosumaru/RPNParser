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
#ifdef RPN_USE_JIT
		asmjit::X86XmmVar Compile(asmjit::X86Compiler& c) override
		{
			using namespace asmjit;
			auto token = _token->Compile(c);

			auto minus = c.newXmmSs();
			setXmmVariable(c, minus, -1.0f);
			c.mulss(token, minus);
			return token;
		}
#endif
	};
    

	class BinaryOperator : public Operator
	{
	public:
	
		int precedence() override { return 2; }

		bool constant() override { return (_tokens[0] && !_tokens[0]->constant()) || (_tokens[1] && !_tokens[1]->constant()) ? false : true; }

	protected:
		void Parse(ParserContext &tokens) override
		{
			_tokens[1] = tokens.popAndParseToken();
			_tokens[0] = tokens.popAndParseToken();
		}
		
#ifdef RPN_USE_JIT
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
#endif

		float value_of(unsigned index) { return _tokens[index]->value(); }

		TokenPtr _tokens[2];
	};


	class BinaryPlusOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) + value_of(1); }

#ifdef RPN_USE_JIT
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.addss(o1, o2);
			return o1;
		}
#endif
	};

	class BinaryMinusOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) - value_of(1); }

#ifdef RPN_USE_JIT
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.subss(o1, o2);
			return o1;
		}
#endif
	};

	class BinaryMultiplyOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) * value_of(1); }
		int precedence() override { return 3; }

#ifdef RPN_USE_JIT
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.mulss(o1, o2);
			return o1;
		}
#endif
	};

	class BinaryDivisionOperator : public BinaryOperator
	{
	public:
		float value() override { return value_of(0) / value_of(1); }
		int precedence() override { return 3; }

#ifdef RPN_USE_JIT
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.divss(o1, o2);
			return o1;
		}
#endif
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

#ifdef RPN_USE_JIT
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			auto out = c.newXmmSs();
			setXmmVariable(c, out, 1.0f);
			c.cmpss(o1, o2, (int)_imm);
			c.andps(out, o1);
			return out;
		}
#endif

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

#ifdef RPN_USE_JIT
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.andps(o1, o2);

			auto zero = c.newXmmSs();
			setXmmVariable(c, zero, 0.0f);
			c.cmpss(o1, zero, 4); //o1 != zero

			setXmmVariable(c, zero, 1.0f); //well, abuse zero
			c.andps(o1, zero);
			return o1;
		}
#endif
	};

	class BinaryOrOperator : public BinaryOperator
	{
	public:
		float value() override { return (value_of(0) || value_of(1)) ? 1.0f : 0.0f; }
		int precedence() override { return 14; }

#ifdef RPN_USE_JIT
		asmjit::X86XmmVar BinaryCompile(asmjit::X86Compiler& c, asmjit::X86XmmVar& o1, asmjit::X86XmmVar &o2) override
		{
			c.orps(o1, o2);

			auto zero = c.newXmmSs();
			setXmmVariable(c, zero, 0.0f);
			c.cmpss(o1, zero, 4); //o1 != zero

			setXmmVariable(c, zero, 1.0f); //well, abuse zero
			c.andps(o1, zero);
			return o1;
		}
#endif
	};


}

#endif