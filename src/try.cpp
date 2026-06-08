#include "thread_safe_queue.h"
#include <iostream>
#include <thread>
int main(){
	threadsafe_queue q;
	std::thread t1(&threadsafe_queue::run, &q);
	std::thread t2(&threadsafe_queue::run, &q);
	t1.join();
	t2.join();

	std::cout << q.size() << std::endl;
	return 0;
}
