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

std::unique_ptr<Token> ParserContext::popAndParseToken()
{
	if (output.empty())
    {
        error = true;
		return nullptr;
    }
	auto op = std::move(output.back());
	output.pop_back();
	op->Parse(*this);

/*
#ifndef _DEBUG
	//optimize, cull tree
	if (op->type() != Token::Type::Variable && op->constant())
		op.reset(new Value(op->value()));
#endif
	*/
	return op;
}

void ParserContext::AddToken(Token* token)
{
    last_operator_type = token->type();
	if (token->type() == Token::Type::Variable)
	{
		//If the token is a number, then add it to the output queue
		output.emplace_back(token);
		return;
	}

	if (token->type() == Token::Type::Function)
	{
		//If the token is a function token, then push it onto the stack.
		operator_stack.push(std::move(TokenPtr(token)));
		return;
	}

	if (token->type() == Token::Type::FunctionArgumentSeparator)
	{
		//Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
		//If no left parentheses are encountered, either the separator was misplaced or parentheses were mismatched.

		while (!operator_stack.empty() && operator_stack.top()->type() != Token::Type::LeftParenthesis)
		{
			output.emplace_back(std::move(operator_stack.top()));
			operator_stack.pop();
		}

		return;
	}

	if (token->type() == Token::Type::Operator)
	{
		//while there is an operator token, o2, at the top of the stack, and
		while (!operator_stack.empty())
		{
			auto &o1 = token;
			auto &o2 = operator_stack.top();

			//either o1 is left-associative and its precedence is equal to that of o2,
			//or o1 has precedence less than that of o2,
			if ((o1->left_associative() && o1->precedence() == o2->precedence()) || (o1->precedence() < o2->precedence()))
			{
				//pop o2 off the stack, onto the output queue;
				output.emplace_back(std::move(operator_stack.top()));
				operator_stack.pop();
				continue;
			}
			break;
		}

		//push o1 onto the stack.
		operator_stack.push(std::move(TokenPtr(token)));
		return;
	}

	if (token->type() == Token::Type::LeftParenthesis)
	{
		//If the token is a left parenthesis, then push it onto the stack.
		operator_stack.push(std::move(TokenPtr(token)));
		return;
	}

	if (token->type() == Token::Type::RightParenthesis)
	{
		//Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
		while (!operator_stack.empty())
		{
			auto &o = operator_stack.top();

			if (o->type() != Token::Type::LeftParenthesis)
			{
				output.emplace_back(std::move(operator_stack.top()));
				operator_stack.pop();
				continue;
			}
			break;
		}

		if (operator_stack.empty())
		{
			//ERROR
		}

		//Pop the left parenthesis from the stack, but not onto the output queue.
		operator_stack.pop();

		//If the token at the top of the stack is a function token, pop it onto the output queue.
		if (!operator_stack.empty() && operator_stack.top()->type() == Token::Type::Function)
		{
			output.emplace_back(std::move(operator_stack.top()));
			operator_stack.pop();
		}

		return;
	}

}



void _InitializeParser()
{
	Functions::AddStatelessLambda("if", [](float c, float a, float b) { return c != 0.0f ? a : b; });
	Functions::AddStatelessLambda("math.max", [](float a, float b) { return a > b ? a : b; });
	Functions::AddStatelessLambda("math.min", [](float a, float b) { return a > b ? b : a; });
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

Parser::FunctionPtr Parser::Compile(const std::string& text)
{
	using namespace asmjit;
	auto token = Parse(text);

	if (!token)
		return nullptr;

	if (!token->compilable())
		return nullptr;

	static JitRuntime runtime; //this could be better
	StringLogger logger;
	X86Compiler c(&runtime);
	c.setLogger(&logger);


	c.addFunc(kFuncConvHost, FuncBuilder0<float>());
	c.ret(token->Compile(c));
	c.endFunc();


	return asmjit_cast<FunctionPtr>(c.make());
}
