#ifndef MUTEX_QUEUE_HH
#define MUTEX_QUEUE_HH

#include <iostream>
#include <vector>
#include <optional>
#include <queue>
#include <mutex>
struct mutex_queue{
	std::queue<int> queue;
	std::mutex lock;

	mutex_queue(){};
	void enque(int);
	std::optional<int> deque();
	void run();
	size_t size();
};
#endif
