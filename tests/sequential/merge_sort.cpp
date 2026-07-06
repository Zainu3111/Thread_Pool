#include <iostream>
#include "../../benchmarks/merge_sort.h"
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include "../../src/universal.h"

int main(){

	const int BENCHMARK = 1000000;
	unsigned int SEED = 31;
	std::mt19937 gen(SEED);
	std::uniform_int_distribution<int> distrib(1, 100000000);
	std::string benchmark = "Sequential Merge-Sort";

	for(int i{1}; i <= 30; ++i){
		std::cout << "Iteration Number: " << i << std::endl;
		std::vector<int> arr(BENCHMARK * i);
		std::generate(arr.begin(), arr.end(), [&](){
				return distrib(gen);
				});

		auto start = std::chrono::steady_clock::now();

		// start code
		serial_merge_sort(arr, 0, arr.size() - 1);	
		// end code
		auto end = std::chrono::steady_clock::now();

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		record_result(benchmark, BENCHMARK * i, 1, ms.count());
//		std::cout << "Total Time: " << ms.count() << " ms\n";
	}
	return 0;
}
