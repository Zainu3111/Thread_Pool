#include "./basic_threadpool.h"
#include <iostream>
#include "../../benchmarks/Prime.h"
#include <chrono>
#define BENCHMARK 1000000
void run(){
	thread_pool pool;
	std::cout << "Testing threadpool" << std::endl;
	for (int i{}; i < 100; ++i){
		pool.submit([i](){

				});
	}
}
int main(){
	auto start = std::chrono::steady_clock::now();

	// start code
	run();	
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Total Primes: " << res << std::endl;
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}

