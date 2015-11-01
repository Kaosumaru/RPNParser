#ifndef MXRPNPARSER
#define MXRPNPARSER

#include "Token.h"
#include <vector>

#include <sstream>
#include <functional>
#include <stack>
#include <cassert>

namespace RPN
{
	class Parser
	{
	public:
		using Context = ParserContext;
		using ParsingRule = std::function<bool(Context &context)>;
		using FunctionPtr = float(*)();

		class CompiledFunction
		{
		public:
			CompiledFunction() {}
			CompiledFunction(const FunctionPtr &f, TokenPtr&& t) : _function(f), _token(std::move(t)) {}

			operator bool() const
			{
				return _function != nullptr;
			}

			float operator()()
			{
				return _function();
			}

			void Release();
		protected:
			FunctionPtr _function = nullptr;
			TokenPtr    _token;
		};

		Parser();

		TokenPtr Parse(const std::string& text)
		{
			Context context;
			context.input.str(text);

			while (true)
			{
				context.input.peek();
				if (context.error || context.input.eof())
					break;


				bool parsed = false;

				for (auto &rule : _rules)
					if (rule(context))
					{
						parsed = true;
						break;
					}

				if (!parsed)
				{
					context.error = true;
					break;
				}
			}

			while (!context.operator_stack.empty())
			{
				auto &op = context.operator_stack.top();
				if (op->type() == Token::Type::LeftParenthesis || op->type() == Token::Type::RightParenthesis)
				{
					return nullptr;
				}

				context.output.emplace_back(std::move(op));
				context.operator_stack.pop();
			}

			auto ret = context.popAndParseToken();
			if (!ret || context.error || !context.output.empty())
			{
#ifdef _DEBUG
				assert(false);
#endif
				return nullptr;
			}
			return ret;
		}

		CompiledFunction Compile(const std::string& text);


		static Parser& Default()
		{
			static Parser p;
			return p;
		}


	protected:
		std::vector<ParsingRule> _rules;
	};

}

#endif
