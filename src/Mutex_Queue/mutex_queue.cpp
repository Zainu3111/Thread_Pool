#include "mutex_queue.h"
#include <cstdlib>

void mutex_queue::enque(int x){
	std::lock_guard<std::mutex> gaurd(lock);
	queue.push(x);
}

std::optional<int> mutex_queue::deque(){
	std::lock_guard<std::mutex> guard(lock);
	if (queue.size() == 0){
		return {};
	}
	int x = queue.front();
	queue.pop();
	return x;
}

void mutex_queue::run(){
	for(size_t i{}; i < 100000; ++i){
		int x = rand() % 100 + 1;  // [1, 100]
		enque(x);
	}
}

size_t mutex_queue::size(){
	return queue.size();
}
