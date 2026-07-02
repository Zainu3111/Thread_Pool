#include <iostream>
#include "../../benchmarks/fib.h"
#include <chrono>
#define BENCHMARK 45

int main(){
	int n = 40;
	auto start = std::chrono::steady_clock::now();

	// start code
	std::cout << "Testing Sequential Code for Fibonacci." << std::endl;
	int res = seq_fib(BENCHMARK);
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
//	std::cout << end.count() << std::endl;
	std::cout << "fib " << n << ": " << res << std::endl;
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}

