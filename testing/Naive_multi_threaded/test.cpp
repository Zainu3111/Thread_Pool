#include <iostream>
#include "../../benchmarks/Prime.h"
#include <chrono>
#include <thread>
#include <vector>
#define BENCHMARK 1000000

size_t test(size_t start, size_t end){
	size_t res{};
	for (size_t i{start}; i <= end; ++i){
		if (is_prime(i)){
			++res;
		}
	}
	return res;
}

int main(){
	auto start = std::chrono::steady_clock::now();

	// start code
	const int THREAD_COUNT = std::thread::hardware_concurrency();
	size_t window = BENCHMARK / THREAD_COUNT;
	std::vector<size_t> res(THREAD_COUNT);
	std::vector<std::thread> thread_arr(THREAD_COUNT); 
	for (size_t i{}; i < THREAD_COUNT; ++i){
		thread_arr[i] = std::thread([&, i]{
				res[i] = test(i * window, (i + 1) * window);
				});
	}
	size_t result{};
	for (size_t i{}; i < THREAD_COUNT; ++i){
		thread_arr[i].join();
		result += res[i];
	}	

	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Total Primes: " << result << std::endl;
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}

