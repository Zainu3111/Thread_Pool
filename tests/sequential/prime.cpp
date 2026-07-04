#include "../../benchmarks/prime.h"
#include <chrono>
#include "../../src/universal.h"
#include <iostream>

int main(){

	constexpr size_t BENCHMARK = 1000000;
	size_t res{};
	size_t i{0};
	constexpr size_t iterations = 50000000 / BENCHMARK;
	std::string benchmark = "Sequential Prime";
	for(size_t j{1}; j <= iterations; ++j){
		std::cout << "Iteration Number:" << j << std::endl;
		auto start = std::chrono::steady_clock::now();
		res = 0;
		for(i = 0; i <= (j * BENCHMARK); ++i){
			if (is_prime(i)){
				++res;
			}
		}
		auto end = std::chrono::steady_clock::now();

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		record_result(benchmark, j * BENCHMARK, 1, ms.count());
	}

	return 0;
}
