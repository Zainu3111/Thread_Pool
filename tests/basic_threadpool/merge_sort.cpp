#include <iostream>
#include "./merge_sort.h"
#include "./threadpool.h"
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include "../../src/universal.h"

void run(std::vector<int>& arr){
	thread_pool pool;
	//std::cout << "Testing Merge-Sort With a Global Lock-Based Queue Threadpool" << std::endl;
	auto task = pool.submit([&pool, &arr](){
			parallel_merge_sort(pool, arr, 0, arr.size() - 1);
			});
	task.wait();
}


int main(){
	const int BENCHMARK = 1000000;
	unsigned int SEED = 31;
	std::mt19937 gen(SEED);
	std::uniform_int_distribution<int> distrib(1, 100000000);
	std::string benchmark = "Work-Stealing Merge-Sort";
	const int THREAD_COUNT = std::thread::hardware_concurrency();
	for(int i{1}; i <= 30; ++i){
		std::cout << "Iteration Number: " << i << std::endl;
		std::vector<int> arr(BENCHMARK * i);
		std::generate(arr.begin(), arr.end(), [&](){
				return distrib(gen);
				});

		auto start = std::chrono::steady_clock::now();

		// start code
		run(arr);	
		// end code
		auto end = std::chrono::steady_clock::now();
	
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		record_result(benchmark, BENCHMARK * i, THREAD_COUNT, ms.count());
		std::cout << "Time Taken: " << ms.count() << "ms" << std::endl; 
	}
	return 0;
}
