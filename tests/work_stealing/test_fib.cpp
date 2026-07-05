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
	std::cout << val.get() << std::endl;
}

int main(){

	auto start = std::chrono::steady_clock::now();
	// start code
	run(45);
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	
	return 0;
}

