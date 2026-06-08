#include "basic_queue.h"
#include <cstdlib>

void basic_queue::enque(int x){
	queue.push(x);
}

std::optional<int> basic_queue::deque(){
	if (queue.size() == 0){
		return {};
	}
	int x = queue.front();
	queue.pop();
	return x;
}

void basic_queue::run(){
	for(size_t i{}; i < 100000; ++i){
		int x = rand() % 100 + 1;  // [1, 100]
		enque(x);
	}
}

size_t basic_queue::size(){
	return queue.size();
}
