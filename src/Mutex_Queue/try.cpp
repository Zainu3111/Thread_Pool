#include "mutex_queue.h"
#include <iostream>
#include <thread>
int main(){
	mutex_queue q;
	std::thread t1(&mutex_queue::run, &q);
	std::thread t2(&mutex_queue::run, &q);
	t1.join();
	t2.join();

	std::cout << q.size() << std::endl;
	return 0;
}
