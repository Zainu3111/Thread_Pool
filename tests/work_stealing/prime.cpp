#include "../../src/universal.h"
#include "../../src/threadpool.h"
#include <iostream>
#include "../../benchmarks/prime.h"
#include <chrono>
//#define BENCHMARK 48000000
#define TEST_SIZE 10000
#include <unordered_map>
size_t test(size_t start, size_t end){
	size_t res{};
	for (size_t i{start}; i <= end; ++i){
		if (is_prime(i)){
			++res;
		}
	}
	return res;
}

void run(std::unordered_map<std::thread::id, size_t>& map, size_t BENCHMARK){
	thread_pool pool;
	std::mutex mux;
//	std::cout << "Testing threadpool" << std::endl;
	size_t ciel = BENCHMARK / TEST_SIZE;
	for (size_t i{}; i < ciel; ++i){
		pool.submit([i, &map, &mux](){
				size_t cur = test(i * TEST_SIZE, (i + 1) * TEST_SIZE);
				std::lock_guard lock_gaurd(mux);
				map[std::this_thread::get_id()] += cur;
				});
	}
}

int main(){
	std::string benchmark = "Work-Stealing Prime";
	constexpr size_t BENCHMARK = 1000000;
	constexpr size_t iterations = 50000000 / BENCHMARK;
	const int THREAD_COUNT = std::thread::hardware_concurrency();
	for(size_t i{}; i < iterations; ++i){ 
		std::cout << "Iteration Number: " << i << std::endl;
		auto start = std::chrono::steady_clock::now();
		// start code
		std::unordered_map<std::thread::id, size_t> map;
		run(map, i * BENCHMARK);
		size_t res{};
		for(auto const& [id, val] : map){
			res += val;
		}
		// end code
		auto end = std::chrono::steady_clock::now();	
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		record_result(benchmark, i * BENCHMARK, THREAD_COUNT, ms.count());
		}
	return 0;
}

