#include <iostream>
#include <future>
#include "../src/Thread_Safe_Queue/thread_safe_queue.h"
inline int seq_fib(int n){
	if (n < 2){
		return n;
	}
	return seq_fib(n - 1) + seq_fib(n-2);
}

inline int parallel_fib(threadsafe_queue<std::packaged_task<int()>>& pool, int n){
	if (n < 30){
		return seq_fib(n);
	}
	std::packaged_task<int()> task1([&pool, n](){
		return parallel_fib(pool, n - 1);
			});
	std::packaged_task<int()> task2([&pool, n](){
		return parallel_fib(pool, n - 2);
			});
	pool.submit(task1);
	pool.submit(task2);

	return task1.get() + task2.get();
}
