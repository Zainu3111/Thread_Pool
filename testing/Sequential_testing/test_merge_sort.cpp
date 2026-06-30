#include <iostream>
#include "../../benchmarks/merge_sort.h"
#include <chrono>
#define BENCHMARK 100000000
#include <vector>
#include <random>
#include <algorithm>

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
	serial_merge_sort(arr, 0, arr.size() - 1);	
	// end code
	auto end = std::chrono::steady_clock::now();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Total Time: " << ms.count() << " ms\n";
	return 0;
}
