#include "../../src/threadpool.h"
#include <iostream>
#include "../../benchmarks/fib.h"
#include <chrono>
#define BENCHMARK 45 

void run(){
	thread_pool pool;
	std::cout << "Testing Threadpool with Work-Stealing Local-Queue" << std::endl;
	auto val = pool.submit([&pool](){
			return parallel_fib(pool, BENCHMARK);
			});
	std::cout << "fib of " << BENCHMARK << " = " << val.get() << std::endl;
}

int main(){
	auto start = std::chrono::steady_clock::now();

	// start code
	std::unordered_map<std::thread::id, size_t> map;
	run();
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}

