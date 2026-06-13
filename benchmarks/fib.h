#include <iostream>

inline int seq_fib(int n){
	if (n < 2){
		return n;
	}
	return seq_fib(n - 1) + seq_fib(n-2);
}

inline void parallel_fib(int n){
	if (n < 30){
		seq_fib(n);
	}
}
