#include "Parser.h"
#include "Operator.h"
#include "Function.h"
#include "MiscTokens.h"
#include "Utils.h"
#include <map>

using namespace RPN;

std::map<std::string, std::function<Token*(Parser::Context &context)>> Functions::_functions;


namespace RPN
{
	namespace impl
	{
		asmjit::JitRuntime& jitRuntime()
		{
			static asmjit::JitRuntime runtime;
			return runtime;
		}
		
	};


	bool rule_space_eater(Parser::Context &context)
	{
		auto peek = context.input.peek();
		if (peek != ' ')
			return false;

		while (true)
		{
			auto peek = context.input.peek();
			if (peek != ' ')
				return true;
			context.input.get();
		}
	}


	bool rule_value(Parser::Context &context)
	{
		auto peek = context.input.peek();
		if (!isdigit(peek))
			return false;

		float value = 0.0f;
		context.input >> value;

		context.AddToken(new Value(value));
		return true;
	}

	bool rule_string(Parser::Context &context)
	{
		auto peek = context.input.peek();
		if (peek != '\'')
			return false;

		context.input.get();
		auto charsAllowedInString = [](char c, int index) { return c != '\''; };

		std::string value = eat_string_in_stream(context.input, charsAllowedInString);
		context.input.get();

		context.AddToken(new StringValue(value));
		return true;
	}

	bool rule_short_operator(Parser::Context &context)
	{
		auto peek = context.input.peek();

		static std::map<char, std::function<Token*(Parser::Context &context)>> charToToken =
		{
			{ '+', [](Parser::Context &context) { return new BinaryPlusOperator; } },
			{ '-', [](Parser::Context &context) -> Token*
                {
                    if (context.last_operator_type == Token::Type::None || context.last_operator_type == Token::Type::LeftParenthesis || context.last_operator_type == Token::Type::Operator)
                        return new UnaryMinusOperator;
                    return new BinaryMinusOperator;
                } },
			{ '*', [](Parser::Context &context) { return new BinaryMultiplyOperator; } },
			{ '/', [](Parser::Context &context) { return new BinaryDivisionOperator; } },
			{ '(', [](Parser::Context &context) { return new LeftParenthesis; } },
			{ ')', [](Parser::Context &context) { return new RightParenthesis; } },
			{ ',', [](Parser::Context &context) { return new Comma; } },

			{ '<', [](Parser::Context &context) { return new BinaryLesserThanOperator; } },
			{ '>', [](Parser::Context &context) { return new BinaryGreaterThanOperator; } },
		};

		auto it = charToToken.find(peek);
		if (it == charToToken.end())
			return false;
		context.input.get();
		context.AddToken(it->second(context));
		return true;
	}




	bool rule_long_operator(Parser::Context &context)
	{
		static std::map<std::string, std::function<Token*(Parser::Context &context)>> strToToken =
		{
			{ "==", [](Parser::Context &context) { return new BinaryEqualsOperator; } },
			{ "!=", [](Parser::Context &context) { return new BinaryNotEqualsOperator; } },
			{ ">=", [](Parser::Context &context) { return new BinaryGreaterOrEqualsOperator; } },
			{ "<=", [](Parser::Context &context) { return new BinaryLesserOrEqualsOperator; } },

			{ "&&", [](Parser::Context &context) { return new BinaryAndOperator; } },
			{ "||", [](Parser::Context &context) { return new BinaryOrOperator; } },
		};



		for (auto &pair : strToToken)
		{
			if (!eat_string_in_stream_if_equal(context.input, pair.first))
				continue;
			context.AddToken(pair.second(context));
			return true;
		}

		return false;
	}


	bool embedded_function(Parser::Context &context)
	{
		auto position = context.input.tellg();
		auto charsAllowedInFuncName = [](char c, int index) { return isalpha(c) || (index == 0 && isdigit(c)) || (index != 0 && c == '.') || c == '_'; };
		std::string str = eat_string_in_stream(context.input, charsAllowedInFuncName);

		auto token = Functions::getFunction(str, context);
		if (token)
		{
			context.AddToken(token);
			return true;
		}

		context.input.seekg(position);
		return false;
	}



}


void Parser::CompiledFunction::Release()
{
	auto& runtime = impl::jitRuntime();
	runtime.release((void*)_function);
}


void _InitializeParser()
{
	Functions::AddStatelessLambda("if", [](float c, float a, float b) { return c != 0.0f ? a : b; });
	Functions::AddStatelessLambda("math.max", [](float a, float b) { return a > b ? a : b; });
	Functions::AddStatelessLambda("math.min", [](float a, float b) { return a > b ? b : a; });

	{
		Functions::AddLambda("string.length", [](const std::string &str) { return (float)str.size(); });
		Functions::AddLambda("string.join", [](const std::string &str1, const std::string &str2) { return str1+str2; });
	}
	

	{
		static std::stack<float> stack;

		auto push = [&](float arg) 
		{ 
			stack.push(arg); return arg; 
		};
		auto pop = [&]()
		{ 
			if (stack.empty())
			{
				assert(false);
				return 0.0f;
			}
			auto a = stack.top();  
			stack.pop(); 
			return a;
		};


		Functions::AddLambda("stack.push", push);
		Functions::AddLambda("stack.pop", pop);
	}


}

Parser::Parser()
{
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		_InitializeParser();
	}

	_rules.push_back(rule_space_eater);
	_rules.push_back(rule_value);
	_rules.push_back(rule_string);
	_rules.push_back(rule_long_operator);
	_rules.push_back(rule_short_operator);
	_rules.push_back(embedded_function);

}

Parser::CompiledFunction Parser::Compile(const std::string& text)
{
	using namespace asmjit;
	auto token = Parse(text);

	if (!token)
		return {};


	auto& runtime = impl::jitRuntime();
	StringLogger logger;
	X86Compiler c(&runtime);
	c.setLogger(&logger);


	c.addFunc(kFuncConvHost, FuncBuilder0<float>());
	c.ret(token->Compile(c));
	c.endFunc();

	auto pointer = asmjit_cast<FunctionPtr>(c.make());

	return{ pointer , std::move(token) };
}
