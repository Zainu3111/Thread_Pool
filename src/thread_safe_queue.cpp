#include "thread_safe_queue.h"

void threadsafe_queue::enque(int x){
	queue.push(x);
}

std::optional<int> threadsafe_queue::deque(){
	if (queue.size() == 0){
		return {};
	}
	int x = queue.front();
	queue.pop();
	return x;
}
