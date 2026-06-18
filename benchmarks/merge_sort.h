#include <iostream>
#include <vector>
#include "../src/threadpool.h"
#include <future>

inline void merge(std::vector<int>& arr, size_t l, size_t mid, size_t r){
	size_t n1{mid - l + 1}, n2{r - mid};
	std::vector<int> tmp1(n1);
	std::vector<int> tmp2(n2);
	for(size_t i{}; i < n1; ++i) tmp1.at(i) = arr.at(l + i); 
	for(size_t i{}; i < n2; ++i) tmp2.at(i) = arr.at(mid + 1 + i);
	size_t p1{}, p2{}, pos{l};
	while(p1 < n1 && p2 < n2){
		if(tmp1.at(p1) >= tmp2.at(p2)){
			arr.at(pos++) = tmp1.at(p1++);
		}else{
			arr.at(pos++) = tmp2.at(p2++);
		}
	}
	while(p1 < n1){
		arr.at(pos++) = tmp1.at(p1++);
	}
	while(p2 < n2){
		arr.at(pos++) = tmp2.at(p2++);
	}
}

inline void serial_merge_sort(std::vector<int>& arr, size_t l, size_t r){
		if(l >= r){
			return;
		}
		size_t mid{(l + r) / 2};
		serial_merge_sort(arr, l, mid);
		serial_merge_sort(arr, mid + 1, r);
		merge(arr, l, mid, r);
}

inline void parallel_merge_sort(thread_pool& pool, std::vector<int>& arr, size_t l, size_t r){
		if( r - l <= 1000 ) return serial_merge_sort(arr, l, r);
		if(l >= r){
			return;
		}
		size_t mid{(l + r) / 2};
		std::packaged_task<void()> task1([&]() {
				parallel_merge_sort(pool, arr, l, mid);
		});
		std::packaged_task<void()> task2([&]() {
				parallel_merge_sort(pool, arr, mid + 1, r);
		});
		auto t1 = task1.get_future();
		auto t2 = task2.get_future();

		pool.submit(std::move(task1));
		pool.submit(std::move(task2));
		serial_merge_sort(arr, mid + 1, r);
		merge(arr, l, mid, r);
}

