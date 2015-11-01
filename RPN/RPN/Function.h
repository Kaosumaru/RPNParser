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



		template<typename F> auto make_function_pointer(F &&f) {
			using PtrType = typename std::add_pointer<typename get_signature_impl<F>::type>::type;
			return (PtrType) f;
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
		class RPNToType
		{
		public:


		};

		template<>
		class RPNToType<float>
		{
		public:
			static float from(const TokenPtr& token)
			{
				return token->value();
			}
		};

		template<>
		class RPNToType<std::string>
		{
		public:
			static std::string from(const TokenPtr& token)
			{
				return token->stringValue();
			}
		};


		template<unsigned I, typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl {};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl<0, Ret, Args...> : asmjit::FuncBuilder0<Ret> {};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl<1, Ret, Args...> : asmjit::FuncBuilder1<Ret, Args...> {};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl<2, Ret, Args...> : asmjit::FuncBuilder2<Ret, Args...> {};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl<3, Ret, Args...> : asmjit::FuncBuilder3<Ret, Args...> {};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl<4, Ret, Args...> : asmjit::FuncBuilder4<Ret, Args...> {};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic : public FuncBuilderVariadic_Impl<sizeof...(Args), Ret, Args...>
		{

		};

		template<typename Ret, typename ...Args>
		struct FuncCaller
		{
			using FuncType = Ret(*) (Args...);

			static asmjit::X86XmmVar callFunction(asmjit::X86Compiler &c, FuncType func, const std::vector<asmjit::X86XmmVar>& args)
			{
				using namespace asmjit;
				X86XmmVar out(c, kX86VarTypeXmmSs);

				auto ctx = c.call((uint64_t)func, kFuncConvHost, FuncBuilderVariadic<Ret, Args...>()); //TODO FuncBuilder2 should be FuncBuilderX
				for (int i = 0; i < args.size(); i++)
					ctx->setArg(i, args[i]);
				ctx->setRet(0, out);


				return out;
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

	template<typename Type>
	class SimpleFunction
	{

	};

	template<typename R, typename... Args>
	class SimpleFunction<R(*)(Args...)> : public Function
	{
	public:
		static const unsigned short int arity = sizeof...(Args);
		using FuncPointer = R(*)(Args...);

		SimpleFunction(const FuncPointer& func) : _func(func)
		{

		}

		void Parse(ParserContext &tokens) override
		{
			_tokens.resize(arity);
			for (auto it = _tokens.rbegin(); it != _tokens.rend(); it++)
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
			return _func(impl::RPNToType<typename std::decay<Args>::type>::from(_tokens[S]) ...);
		}

		static float add(float a, float b)
		{
			return a + b;
		}


		asmjit::X86XmmVar Compile(asmjit::X86Compiler& c) override
		{
			using namespace asmjit;
			
			std::vector<X86XmmVar> arguments;
			arguments.reserve(arity);
			for (auto& token : _tokens)
				arguments.push_back(token->Compile(c));

			auto out = impl::FuncCaller<R, Args...>::callFunction(c, _func, arguments);
			return out;
		}

	protected:
		FuncPointer _func;
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
		static Token* wrapFunction(const T& func)
		{
			return new SimpleFunction<T>(func);
		}

		template<typename T>
		static void AddLambda(const std::string &name, T&& func)
		{
			_functions[name] = [=](Parser::Context &context)
			{
				return Functions::wrapLambda(func);
			};
		}

		template<typename T>
		static void AddStatelessLambda(const std::string &name, T&& func)
		{
			_functions[name] = [=](Parser::Context &context)
			{
				auto p = impl::make_function_pointer(func);
				return wrapFunction(p);
			};
		}

		template<typename T>
		static void AddFunction(const std::string &name, T&& func)
		{
			_functions[name] = [=](Parser::Context &context)
			{
				return wrapFunction(func);
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
