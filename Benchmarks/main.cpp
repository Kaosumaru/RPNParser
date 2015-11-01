#include <iostream>
#include <chrono>
#include "RPN/Parser.h"
#include "RPN/Function.h"
#include "benchpress.hpp"

using namespace std;
benchpress::registration* benchpress::registration::d_this;


void Interpret(benchpress::context* ctx, const std::string& expr)
{
	auto p = RPN::Parser::Default().Parse(expr);
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i) {
		p->value();
	}
}

void Compile(benchpress::context* ctx, const std::string& expr)
{
	auto c = RPN::Parser::Default().Compile(expr);
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i) {
		c();
	}
}

#define BENCHMARK_RPN(x, f) benchpress::auto_register CONCAT2(register_, __LINE__)((CONCAT2("Interpret ", x)), ([](benchpress::context* ctx) {Interpret(ctx, f);})); \
benchpress::auto_register CONCAT2(register2_, __LINE__)((CONCAT2("Compile ", x)), ([](benchpress::context* ctx) {Compile(ctx, f);}));




std::string single_expression = "1";

BENCHMARK_RPN("single_expression", single_expression)


std::string short_expression = "2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2";

BENCHMARK_RPN("short_expression", short_expression)

std::string short_expression2 = "1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1";

BENCHMARK_RPN("short_expression2", short_expression2)

std::string math_function = "math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,1))))))))))))+math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,math.min(1,1))))))))))))";

BENCHMARK_RPN("math function", math_function)

std::string string_function = "string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')+string.length('test')";

BENCHMARK_RPN("string function", string_function)


int main (int argc, char * argv[])
{
	std::chrono::high_resolution_clock::time_point bp_start = std::chrono::high_resolution_clock::now();
	benchpress::options bench_opts;
	benchpress::run_benchmarks(bench_opts);
	float duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now() - bp_start
		).count() / 1000.f;
	std::cout << argv[0] << " " << duration << "s" << std::endl;
	return 0;
}
