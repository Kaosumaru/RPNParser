#include "Token.h"
#include "Operator.h"
#include "Function.h"
#include "MiscTokens.h"
#include "Utils.h"
#include <map>


using namespace RPN;


namespace RPN
{
	namespace impl
	{
		float value_of_token(Token *token)
		{
			return token->value();
		}
	}
}

asmjit::X86XmmVar Token::Compile(asmjit::X86Compiler& c)
{
	using namespace asmjit;

	auto out = c.newXmmSs("OutToken");
	auto arg = c.newIntPtr("PointerToToken");

	c.mov(arg, imm_ptr(this));

	auto ctx = c.call((uint64_t)&impl::value_of_token, FuncBuilder1<float, Token*>(kCallConvHost));
	ctx->setArg(0, arg);
	ctx->setRet(0, out);

	return out;
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


#ifndef RPN_OPTIMIZE_0
	//optimize, cull tree
	if (op->type() != Token::Type::Variable && op->constant())
		op.reset(new Value(op->value()));
#endif

	return op;
}

void ParserContext::AddToken(Token* token)
{
	auto previous_operator_type = last_operator_type;
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

		//push this function on function stack, so we can calculate it's 'call arity'
		//we initialize arity to 1, increment it after we find comma, and decrement if there is no arguments to function 
		//( parenthesis come after self )
		functionArity_stack.push(FunctionArity{ dynamic_cast<Function*>(token), 1 });
		return;
	}

	if (token->type() == Token::Type::FunctionArgumentSeparator)
	{
		//Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
		bool foundParenthesis = false;
		while (true)
		{
			if (operator_stack.empty())
				break;
			if (operator_stack.top()->type() == Token::Type::LeftParenthesis)
			{
				foundParenthesis = true;
				break;
			}

			output.emplace_back(std::move(operator_stack.top()));
			operator_stack.pop();
		}

		//If no left parentheses are encountered, either the separator was misplaced or parentheses were mismatched.
		if (!foundParenthesis)
		{
			error = true;
		}

		//increment arity of function that this belongs to
		if (functionArity_stack.empty() || !functionArity_stack.top().func)
		{
			error = true;
		}
		else
		{
			functionArity_stack.top().arity++;
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

		//If this doesn't belongs to a function call, mark this as empty entry on function arity
		if (previous_operator_type != Token::Type::Function)
			functionArity_stack.push(FunctionArity{ nullptr, 0 });

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
			error = true;
		}

		//Pop the left parenthesis from the stack, but not onto the output queue.
		operator_stack.pop();

		//If the token at the top of the stack is a function token, pop it onto the output queue.
		if (!operator_stack.empty() && operator_stack.top()->type() == Token::Type::Function)
		{
			output.emplace_back(std::move(operator_stack.top()));
			operator_stack.pop();
		}

		//we calculated arity, assign it to function
		if (functionArity_stack.empty())
		{
			error = true;
		}
		else
		{
			auto top = functionArity_stack.top();
			functionArity_stack.pop();
			if (top.func)
			{
				auto arity = top.arity;
				if (previous_operator_type == Token::Type::LeftParenthesis)
					arity--;
				top.func->SetCallArity(arity);
			}
		}
		return;
	}

}