#ifndef MXRPNTOKEN
#define MXRPNTOKEN
#include <memory>
#include <vector>
#include <stack>
#include <sstream>
#include <asmjit/asmjit.h>
#include <cassert>
#include "Utils.h"

namespace RPN
{
	class Parser;
	struct ParserContext;



	class Token
	{
	public:
		friend class Parser;
		enum class Type
		{
            None,
			Variable,
			Operator,
			LeftParenthesis,
			RightParenthesis,
			Function,
			FunctionArgumentSeparator
		};

		enum class VariableType
		{
			Undefined,
			Float,
			String
		};

		Token(){};
		virtual ~Token(){};

		virtual bool constant() { return true; } //returns true if this Token always returns same value
		virtual float value() { return 0.0f; }
		virtual std::string stringValue() { return std::to_string((int)value()); } //TODO fix rounding

		virtual int precedence() { return 0; }
		virtual bool left_associative() { return true; }
		virtual Type type() { return Type::Variable; }
		virtual VariableType returnType() { return VariableType::Float; }

		virtual void Parse(ParserContext &) {}

		virtual asmjit::X86XmmVar Compile(asmjit::X86Compiler& c);
	};

	typedef std::unique_ptr<Token> TokenPtr;

	class Function;

	struct ParserContext
	{
		friend class Parser;


		TokenPtr popAndParseToken();

//TODO
//	protected:
		struct FunctionArity
		{
			FunctionArity() {}
			FunctionArity(Function *f, int a) : func(f), arity(a) {}
			Function *func = nullptr;
			int arity = 0;
		};

		void AddToken(Token* token);
		Token::Type last_operator_type = Token::Type::None;
		std::stack<FunctionArity> functionArity_stack;

		std::stack<TokenPtr> operator_stack;
		std::vector<TokenPtr> output;
		std::stringstream input;
		bool error = false;
	};

	class Value : public Token
	{
	public:
		Value(float value) : _value(value) {}

		float value() override { return _value; }

		VariableType returnType() override { return VariableType::Float; }

		asmjit::X86XmmVar Compile(asmjit::X86Compiler& c) override
		{
			using namespace asmjit;
			auto out = c.newXmmSs();
			setXmmVariable(c, out, _value);
			return out;
		}

	protected:
		float _value;
	};

	class StringValue : public Token
	{
	public:
		StringValue(const std::string& value) : _value(value) {}

		VariableType returnType() override { return VariableType::String; }
		std::string stringValue() override { return _value; }
	protected:
		std::string _value;
	};

}

#endif