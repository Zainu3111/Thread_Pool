#include <iostream>
#include "../../benchmarks/Prime.h"
#include <chrono>
#define BENCHMARK 1000000

int main(){
	auto start = std::chrono::steady_clock::now();

	// start code
	size_t res{};
	for (size_t i{0}; i <= BENCHMARK; ++i){
		if (is_prime(i)){
			++res;
		}
	}
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Total Primes: " << res << std::endl;
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}
