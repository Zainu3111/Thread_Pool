#include <iostream>
#include "../../benchmarks/merge_sort.h"
#include "../../src/Threadpool_With_Local_Deque/threadpool_with_local_queue.h"
#include <chrono>
#define BENCHMARK 10000000
#include <vector>
#include <random>
#include <algorithm>

void run(std::vector<int>& arr){
	thread_pool pool;
	std::cout << "Testing threadpool" << std::endl;
	auto task = pool.submit([&pool, &arr](){
			parallel_merge_sort(pool, arr, 0, arr.size());
			});
	task.wait();
}


int main(){
	std::vector<int> arr(BENCHMARK);
	unsigned int SEED = 31;
	std::mt19937 gen(SEED);
	std::uniform_int_distribution<int> distrib(1, 100000000);
	std::generate(arr.begin(), arr.end(), [&](){
			return distrib(gen);
			});

	auto start = std::chrono::steady_clock::now();

	// start code
	run(arr);	
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}
