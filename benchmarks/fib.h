#include <iostream>
#include <future>
#include "../src/threadpool.h"
inline int seq_fib(int n){
	if (n < 2){
		return n;
	}
	return seq_fib(n - 1) + seq_fib(n-2);
}

inline int parallel_fib(thread_pool& pool, int n){
	if (n < 30){
		return seq_fib(n);
	}
	std::packaged_task<int()> task1([&pool, n](){
		return parallel_fib(pool, n - 1);
			});
	auto f1 = task1.get_future();

	std::packaged_task<int()> task2([&pool, n](){
		return parallel_fib(pool, n - 2);
			});
	auto f2 = task2.get_future();

	pool.submit([t = std::move(task1)]() mutable { t(); });
	pool.submit([t = std::move(task2)]() mutable { t(); });

	return f1.get() + f2.get();
}
