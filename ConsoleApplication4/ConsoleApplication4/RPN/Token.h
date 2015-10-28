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

		Token(){};
		virtual ~Token(){};

		virtual bool constant() { return true; } //returns true if this Token always returns same value
		virtual float value() { return 0.0f; }
		virtual std::string stringValue() { return ""; }

		virtual int precedence() { return 0; }
		virtual bool left_associative() { return true; }
		virtual Type type() { return Type::Variable; }

		virtual void Parse(ParserContext &) {}

		virtual bool compilable() { return false; }
		virtual asmjit::X86XmmVar Compile(asmjit::X86Compiler& c) 
		{
			using namespace asmjit;
			assert(false);
			X86XmmVar out(c, kX86VarTypeXmm, "Value");
			setXmmVariable(c, out, value());
			return out;
		}
	};

	typedef std::unique_ptr<Token> TokenPtr;


	struct ParserContext
	{
		friend class Parser;


		TokenPtr popAndParseToken();

//TODO
//	protected:

		void AddToken(Token* token);
		Token::Type last_operator_type = Token::Type::None;
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

		asmjit::X86XmmVar Compile(asmjit::X86Compiler& c) override
		{
			using namespace asmjit;
			X86XmmVar out(c);
			setXmmVariable(c, out, _value);
			return out;
		}

		bool compilable() override { return true; }
	protected:
		float _value;
	};

	class StringValue : public Token
	{
	public:
		StringValue(const std::string& value) : _value(value) {}

		std::string stringValue() override { return _value; }
	protected:
		std::string _value;
	};

}

#endif