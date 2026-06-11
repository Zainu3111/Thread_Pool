#include "./basic_threadpool.h"
#include <iostream>
#include "../../benchmarks/Prime.h"
#include <chrono>
#define BENCHMARK 1000000
#define TEST_SIZE 850
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

void run(std::unordered_map<std::thread::id, size_t>& map){
	thread_pool pool;
	std::mutex mux;
	std::cout << "Testing threadpool" << std::endl;
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
	auto start = std::chrono::steady_clock::now();

	// start code
	std::unordered_map<std::thread::id, size_t> map;
	run(map);
	size_t res{};
	for(auto const& [id, val] : map){
		res += val;
	}
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Total Primes: " << res << std::endl;
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}

