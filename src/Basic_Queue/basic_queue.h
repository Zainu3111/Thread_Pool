#ifndef QUEUE_HH
#define QUEUE_HH

#include <iostream>
#include <vector>
#include <optional>
#include <queue>
struct basic_queue{
	std::queue<int> queue;

	basic_queue(){};
	void enque(int);
	std::optional<int> deque();
	void run();
	size_t size();
};
#endif
