#include "../../src/threadpool.h"
#include <iostream>
#include "../../benchmarks/fib.h"
#include <chrono>
#include "../../src/universal.h"
void run(size_t len){
	thread_pool pool;
	auto val = pool.submit([&pool, len](){
			return parallel_fib(pool, len);
			});
}

int main(){

	const int BENCHMARK = 40;
	const int size = 2;
	const int THREAD_COUNT = std::thread::hardware_concurrency();
	std::string benchmark = "Sequential Fibonacci";
	for(int i{0}; i <= 10; i += size){
		std::cout << "Iteration Number: " << i << std::endl;
		auto start = std::chrono::steady_clock::now();
		// start code
		run(BENCHMARK + i);
		// end code
		auto end = std::chrono::steady_clock::now();

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	//	std::cout << end.count() << std::endl;
		record_result(benchmark, BENCHMARK + i, THREAD_COUNT, ms.count());
	}
	return 0;
}

