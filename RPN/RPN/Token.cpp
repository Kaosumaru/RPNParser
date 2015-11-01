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

	X86XmmVar out(c, kX86VarTypeXmmSs, "OutToken");
	X86GpVar arg(c, kVarTypeIntPtr, "PointerToToken");
	c.mov(arg, imm_ptr(this));

	auto ctx = c.call((uint64_t)&impl::value_of_token, kFuncConvHost, FuncBuilder1<float, Token*>()); //TODO FuncBuilder2 should be FuncBuilderX
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