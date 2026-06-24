#include "../../src/Threadpool_With_Local_Deque/threadpool_with_local_queue.h"
#include <iostream>
#include "../../benchmarks/fib.h"
#include <chrono>
#define BENCHMARK 

void run(){
	thread_pool pool;
	std::cout << "Testing threadpool" << std::endl;
	std::packaged_task<int()> task([&pool](){
			return parallel_fib(pool, BENCHMARK);
			});
	auto val = task.get_future();
	pool.submit(std::move(task));
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

