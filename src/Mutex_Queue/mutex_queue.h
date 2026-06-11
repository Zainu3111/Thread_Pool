#ifndef MUTEX_QUEUE_HH
#define MUTEX_QUEUE_HH

#include <iostream>
#include <vector>
#include <optional>
#include <queue>
#include <mutex>

template <typename T>
struct mutex_queue{
	std::queue<T> queue;
	std::mutex lock;

	mutex_queue(){};
	inline void enque(T x){
		std::lock_guard<std::mutex> gaurd(lock);
		queue.push(x);
	}

	std::optional<T> deque(){
		std::lock_guard<std::mutex> guard(lock);
		if (queue.empty()){
			return {};
		}
		T x = queue.front();
		queue.pop();
		return x;
	}
	size_t size(){
		std::lock_guard<std::mutex> guard(lock);
		return queue.size();
	}
	bool is_empty(){
		std::lock_guard<std::mutex> guard(lock);
		return queue.size() == 0;
	}
};
#endif
