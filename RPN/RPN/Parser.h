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
		typedef std::function<bool(Context &context)> ParsingRule;


		Parser();

		TokenPtr Parse(const std::string& text)
		{
			Context context;
			context.input.str(text);

			while (true)
			{
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
			assert(ret);
			return ret;
		}


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
