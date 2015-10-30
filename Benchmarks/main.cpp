#include <iostream>
#include <chrono>
#include "RPN/Parser.h"
#include "RPN/Function.h"
#include "benchpress.hpp"

using namespace std;
benchpress::registration* benchpress::registration::d_this;

std::string short_expression = "2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2";

BENCHMARK("Interpreted value", [](benchpress::context* ctx) {
	auto p = RPN::Parser::Default().Parse(short_expression);
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i) {
		p->value();
	}
})

BENCHMARK("Compiled value", [](benchpress::context* ctx) {
	auto c = RPN::Parser::Default().Compile(short_expression);
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i) {
		c();
	}
})

std::string short_expression2 = "1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1==1";

BENCHMARK("Interpreted value", [](benchpress::context* ctx) {
	auto p = RPN::Parser::Default().Parse(short_expression2);
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i) {
		p->value();
	}
})

BENCHMARK("Compiled value", [](benchpress::context* ctx) {
	auto c = RPN::Parser::Default().Compile(short_expression2);
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i) {
		c();
	}
})


float add(float a, float b)
{
	return a + b;
}



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
