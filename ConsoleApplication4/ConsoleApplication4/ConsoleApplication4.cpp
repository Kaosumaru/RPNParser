// ConsoleApplication4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <vector>
#include <memory>
#include <map>
#include <asmjit/asmjit.h>
#include "RPN/Parser.h"

using namespace asmjit;


void TestAddFunction()
{
	// Create JitRuntime and X86 Compiler.
	JitRuntime runtime;
	X86Compiler c(&runtime);

	// Build function having two arguments and a return value of type 'int'.
	// First type in function builder describes the return value. kFuncConvHost
	// tells compiler to use a host calling convention.
	c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>());

	// Create 32-bit variables (virtual registers) and assign some names to
	// them. Using names is purely optional and only greatly helps while
	// debugging.
	X86GpVar a(c, kVarTypeInt32, "a");
	X86GpVar b(c, kVarTypeInt32, "b");

	// Tell asmjit to use these variables as function arguments.
	c.setArg(0, a);
	c.setArg(1, b);

	// a = a + b;
	c.add(a, b);

	// Tell asmjit to return 'a'.
	c.ret(a);

	// Finalize the current function.
	c.endFunc();

	// Now the Compiler contains the whole function, but the code is not yet
	// generated. To tell compiler to generate the function make() has to be
	// called.

	// Make uses the JitRuntime passed to Compiler constructor to allocate a
	// buffer for the function and make it executable.
	void* funcPtr = c.make();

	// In order to run 'funcPtr' it has to be casted to the desired type.
	// Typedef is a recommended and safe way to create a function-type.
	typedef int(*FuncType)(int, int);

	// Using asmjit_cast is purely optional, it's basically a C-style cast
	// that tries to make it visible that a function-type is returned.
	FuncType func = asmjit_cast<FuncType>(funcPtr);

	// Finally, run it and do something with the result...
	int x = func(1, 2);
	printf("x=%d\n", x); // Outputs "x=3".

						 // The function will remain in memory after Compiler is destroyed, but
						 // will be destroyed together with Runtime. This is just simple example
						 // where we can just destroy both at the end of the scope and that's it.
						 // However, it's a good practice to clean-up resources after they are
						 // not needed and using runtime.release() is the preferred way to free
						 // a function added to JitRuntime.
	runtime.release((void*)func);

	// Runtime and Compiler will be destroyed at the end of the scope.
}

int return6()
{
	return 6;
}

void TestCallFunction()
{
	// Create JitRuntime and X86 Compiler.
	JitRuntime runtime;
	X86Compiler c(&runtime);


	c.addFunc(kFuncConvHost, FuncBuilder0<int>());

	X86GpVar out(c, kVarTypeInt32, "out");
	auto pCallNode = c.call((uint64_t)&return6, kFuncConvHost, FuncBuilder0<int>());
	pCallNode->setRet(0, out);

	c.add(out, 1);
	c.ret(out);
	c.endFunc();

	void* funcPtr = c.make();
	typedef int(*FuncType)();
	FuncType func = asmjit_cast<FuncType>(funcPtr);
	int x = func();
	printf("x=%d\n", x); // Outputs "x=3".

	runtime.release((void*)func);

}


void setXmmVariable(X86Compiler &c, XmmVar &v, float d) {
	X86GpVar temp = c.newGpVar();
	uint64_t *dd = (uint64_t *)(&d);
	c.mov(temp, *dd);
	c.movq(v, temp.m());
	c.unuse(temp);
}

void TestFloat()
{
	// Create JitRuntime and X86 Compiler.
	JitRuntime runtime;
	X86Compiler c(&runtime);


	c.addFunc(kFuncConvHost, FuncBuilder0<float>());

	X86XmmVar out(c, kX86VarTypeXmm, "out");
	setXmmVariable(c, out, 6.0f);

	c.ret(out);
	c.endFunc();

	void* funcPtr = c.make();
	typedef float(*FuncType)();
	FuncType func = asmjit_cast<FuncType>(funcPtr);
	auto x = func();
	printf("x=%f\n", x); // Outputs "x=3".

	runtime.release((void*)func);

}


int _tmain(int argc, _TCHAR* argv[])
{
	TestFloat();

	return 0;
}

