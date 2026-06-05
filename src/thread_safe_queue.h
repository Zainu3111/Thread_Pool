#ifndef QUEUE_HH
#define QUEUE_HH

#include <iostream>
#include <vector>
#include <optional>
#include <queue>
class threadsafe_queue{
	std::queue<int> queue;

	void enque(int);
	std::optional<int> deque();
};
#endif
