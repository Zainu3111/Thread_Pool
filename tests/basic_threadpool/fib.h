#include <iostream>
#include <future>
#include "./threadpool.h"
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
	auto f1 = pool.submit([&pool, n](){
		return parallel_fib(pool, n - 1);
			});
	auto f2 = pool.submit([&pool, n](){
		return parallel_fib(pool, n - 2);
			});

	pool.worker_while_waiting(f1, f2);
	return f1.get() + f2.get();
}
