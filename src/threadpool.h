#ifndef THREADPOOL_HH
#define THREADPOOL_HH


#include<iostream>
#include<vector>
#include<atomic>
#include "./Mutex_Queue/mutex_queue.h"
#include <functional>
#include<thread>
#include <optional>

class join_threads{
	std::vector<std::thread>& threads;

	join_threads(std::vector<std::thread>& t)
		: threads(t)
		 {
		 }
	~join_threads(){
		for(auto& t : threads){
			if(t.joinable()){
				t.join();
			}
		}
	}
};

class thread_pool{
	std::atomic_bool done;
	mutex_queue<std::function<void()>> work_queue;	
	std::vector<std::thread> threads;
	join_threads joiner;

	inline void worker_thread(){
		while(done){
			auto cur = work_queue.deque();
			if (cur.has_value()){
				std::function<void()> task = cur.value();
				task();
			}
			else{
				std::this_thread::yield();
			}
		}
	}
	public:




};


#endif
