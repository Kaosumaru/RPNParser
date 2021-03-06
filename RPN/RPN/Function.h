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

	protected:
		friend struct ParserContext;

		void SetCallArity(int arity) { _callArity = arity; }
		int _callArity = 0;
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
			static Token::VariableType returnType() { return Token::VariableType::Float; }

			static float from(const TokenPtr& token, const std::vector<TokenPtr>& tokens)
			{
				return token->value();
			}
		};

		template<>
		class RPNToType<std::string>
		{
		public:
			static Token::VariableType returnType() { return Token::VariableType::String; }

			static std::string from(const TokenPtr& token, const std::vector<TokenPtr>& tokens)
			{
				return token->stringValue();
			}
		};

		template<>
		class RPNToType<std::vector<TokenPtr>>
		{
		public:
			static const std::vector<TokenPtr>& from(const TokenPtr& token, const std::vector<TokenPtr>& tokens)
			{
				return tokens;
			}
		};

#ifdef RPN_USE_JIT
		template<unsigned I, typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl {};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic_Impl<0, Ret, Args...> : public asmjit::FuncBuilder0<Ret> 
		{
			using asmjit::FuncBuilder0<Ret>::FuncBuilder0;
		};
														   
		template<typename Ret, typename ...Args>		   
		struct FuncBuilderVariadic_Impl<1, Ret, Args...> : public asmjit::FuncBuilder1<Ret, Args...>
		{
			using asmjit::FuncBuilder1<Ret, Args...> ::FuncBuilder1;
		};
														   
		template<typename Ret, typename ...Args>		   
		struct FuncBuilderVariadic_Impl<2, Ret, Args...> : public asmjit::FuncBuilder2<Ret, Args...>
		{
			using asmjit::FuncBuilder2<Ret, Args...> ::FuncBuilder2;
		};
														   
		template<typename Ret, typename ...Args>		   
		struct FuncBuilderVariadic_Impl<3, Ret, Args...> : public asmjit::FuncBuilder3<Ret, Args...>
		{
			using asmjit::FuncBuilder3<Ret, Args...> ::FuncBuilder3;
		};
														   
		template<typename Ret, typename ...Args>		   
		struct FuncBuilderVariadic_Impl<4, Ret, Args...> : public asmjit::FuncBuilder4<Ret, Args...>
		{
			using asmjit::FuncBuilder4<Ret, Args...>::FuncBuilder4;
		};

		template<typename Ret, typename ...Args>
		struct FuncBuilderVariadic : public FuncBuilderVariadic_Impl<sizeof...(Args), Ret, Args...>
		{
			using FuncBuilderVariadic_Impl<sizeof...(Args), Ret, Args...>::FuncBuilderVariadic_Impl;
		};

		template<typename Ret, typename ...Args>
		struct FuncCaller
		{
			using FuncType = Ret(*) (Args...);

			static asmjit::X86XmmVar callFunction(asmjit::X86Compiler &c, FuncType func, const std::vector<asmjit::X86XmmVar>& args)
			{
				using namespace asmjit;
				auto out = c.newXmmSs();

				auto ctx = c.call((uint64_t)func, FuncBuilderVariadic<Ret, Args...>(kCallConvHost));
				for (size_t i = 0; i < args.size(); i++)
					ctx->setArg((uint32_t)i, args[i]);
				ctx->setRet(0, out);


				return out;
			}
		};
#endif
	}

	template<typename R, typename... Args>
	class GenericFunction_Base : public Function
	{
	public:
		static const unsigned short int arity = sizeof...(Args);
		using Functor = std::function < R(Args...) >;

		GenericFunction_Base(const Functor& functor) : _functor(functor)
		{

		}

		Token::VariableType returnType() override
		{
			return impl::RPNToType<R>::returnType();
		}

		void Parse(ParserContext &tokens) override
		{
			if (_callArity < arity)
			{
				tokens.error = true;
			}

			_tokens.resize(_callArity);
			for (auto it = _tokens.rbegin(); it != _tokens.rend(); it++)
			{
				*it = tokens.popAndParseToken();
			}
		}

		template<int ...S>
		R calculateValue(impl::seq<S...>)
		{
			return _functor(impl::RPNToType<typename std::decay<Args>::type>::from(_tokens[S], _tokens) ...);
		}

	protected:
		Functor _functor;
		std::vector<TokenPtr> _tokens;
	};


	template<typename Func>
	class GenericFunction : public Function
	{
	public:


	};


	template<typename... Args>
	class GenericFunction<std::function<float(Args...)>> : public GenericFunction_Base<float, Args...>
	{
	public:
		using Parent = GenericFunction_Base<float, Args...>;
		GenericFunction(const typename Parent::Functor& functor) : GenericFunction_Base<float, Args...>(functor)
		{

		}

		float value() override
		{
			return this->calculateValue(typename impl::gens<Parent::arity>::type());;
		}
	};


	template<typename... Args>
	class GenericFunction<std::function<std::string(Args...)>> : public GenericFunction_Base<std::string, Args...>
	{
	public:
		using Parent = GenericFunction_Base<std::string, Args...>;
		GenericFunction(const typename Parent::Functor& functor) : GenericFunction_Base<std::string, Args...>(functor)
		{

		}

		std::string stringValue() override
		{
			return this->calculateValue(typename impl::gens<Parent::arity>::type());;
		}
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
			if (_callArity < arity)
			{
				tokens.error = true;
			}

			_tokens.resize(_callArity);
			for (auto it = _tokens.rbegin(); it != _tokens.rend(); it++)
			{
				*it = tokens.popAndParseToken();
			}
		}

		Token::VariableType returnType() override
		{
			return impl::RPNToType<R>::returnType();
		}

		float value() override
		{
			return calculateValue(typename impl::gens<arity>::type());;
		}

		template<int ...S>
		R calculateValue(impl::seq<S...>)
		{
			return _func(impl::RPNToType<typename std::decay<Args>::type>::from(_tokens[S], _tokens) ...);
		}

		static float add(float a, float b)
		{
			return a + b;
		}

#ifdef RPN_USE_JIT
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
#endif

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
