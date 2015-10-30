#ifndef MXRPNFUNCTION
#define MXRPNFUNCTION
#include "Token.h"
#include "Parser.h"
#include <functional>
#include <map>

namespace RPN
{

	class Function : public Token
	{
	public:

		Type type() override { return Type::Function; }
		bool constant() override { return false; }
	};


	namespace impl
	{
#ifndef _MSC_VER //not visual studio
        template<typename T> struct remove_class { };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...)> { using type = R(A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

        template<typename T>
        struct get_signature_impl {
            using type = typename remove_class<
            decltype(&std::remove_reference<T>::type::operator())>::type;
        };
        template<typename R, typename... A>
        struct get_signature_impl<R(A...)> { using type = R(A...); };
        template<typename R, typename... A>
        struct get_signature_impl<R(&)(A...)> { using type = R(A...); };
        template<typename R, typename... A>
        struct get_signature_impl<R(*)(A...)> { using type = R(A...); };
        template<typename T> using get_signature = typename get_signature_impl<T>::type;
#else
        template<typename T> struct remove_class { };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...)> { using type = R(typename A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) const> { using type = R(typename A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) volatile> { using type = R(typename A...); };
        template<typename C, typename R, typename... A>
        struct remove_class<R(C::*)(A...) const volatile> { using type = R(typename A...); };

        template<typename T>
        struct get_signature_impl {
            using type = typename remove_class<
            decltype(&std::remove_reference<T>::type::operator())>::type;
        };
        template<typename R, typename... A>
        struct get_signature_impl<R(A...)> { using type = R(typename A...); };
        template<typename R, typename... A>
        struct get_signature_impl<R(&)(A...)> { using type = R(typename A...); };
        template<typename R, typename... A>
        struct get_signature_impl<R(*)(A...)> { using type = R(typename A...); };
        template<typename T> using get_signature = typename get_signature_impl<T>::type;
#endif


		template<typename F> using make_function_type = std::function<get_signature<F>>;
		template<typename F> make_function_type<F> make_function(F &&f) {
			return make_function_type<F>(std::forward<F>(f));
		}


		template<int ...> struct seq { };

		template<int N, int ...S>
		struct gens : gens<N - 1, N - 1, S...> { };

		template<int ...S>
		struct gens<0, S...>
		{
			typedef seq<S...> type;
		};
	}


	namespace impl
	{
		template<typename Type>
		class RPNToType : public Function
		{
		public:


		};

		template<>
		class RPNToType<float> : public Function
		{
		public:
			static float from(const TokenPtr& token)
			{
				return token->value();
			}

			static bool compilable()
			{
				return false;
			}

		};

		template<>
		class RPNToType<std::string> : public Function
		{
		public:
			static std::string from(const TokenPtr& token)
			{
				return token->stringValue();
			}

			static bool compilable()
			{
				return false;
			}
		};

	}

	template<typename Func>
	class GenericFunction : public Function
	{
	public:


	};


	template<typename R, typename... Args>
	class GenericFunction<std::function<R(Args...)>> : public Function
	{
	public:
		static const unsigned short int arity = sizeof...(Args);
		using Functor = std::function < R(Args...) >;

		GenericFunction(const Functor& functor) : _functor(functor)
		{

		}

		void Parse(ParserContext &tokens) override
		{
			_tokens.resize(arity);
			for (auto it = _tokens.rbegin(); it != _tokens.rend(); it ++)
			{
				*it = tokens.popAndParseToken();
			}
		}

		float value() override
		{
			return calculateValue(typename impl::gens<arity>::type());;
		}

		template<int ...S>
		R calculateValue(impl::seq<S...>)
		{
			return _functor(impl::RPNToType<typename std::decay<Args>::type>::from(_tokens[S]) ...);
		}

	protected:
		Functor _functor;
		std::vector<TokenPtr> _tokens;
	};


	class Functions : public Function
	{
	public:
		template<typename T>
		static Token* wrapLambda(T&& func)
		{
			return wrapFunctor(impl::make_function(func));
		}

		template<typename T>
		static Token* wrapFunctor(const std::function<T>& func)
		{
			return new GenericFunction<std::function<T>>(func);
		}

		template<typename T>
		static void AddLambda(const std::string &name, T&& func)
		{
			_functions[name] = [=](Parser::Context &context)
			{
				return Functions::wrapLambda(func);
			};
		}

		static Token* getFunction(const std::string &name, Parser::Context &context)
		{
			auto it = _functions.find(name);
			if (it == _functions.end())
				return nullptr;
			return it->second(context);
		}

	protected:
		static std::map<std::string, std::function<Token*(Parser::Context &context)>> _functions;
	};

}

#endif
