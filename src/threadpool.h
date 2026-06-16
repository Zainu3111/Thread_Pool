#ifndef THREADPOOL_HH
#define THREADPOOL_HH


#include<iostream>
#include<vector>
#include<atomic>
#include "./Thread_Safe_Queue/thread_safe_queue.h"
#include <functional>
#include <thread>
#include<condition_variable>

class thread_pool{
	std::atomic_bool done;
	threadsafe_queue<std::function<void()>> global_work_queue;	
	std::vector<std::thread> threads;

	void worker_thread(){
		while(true){
			std::function<void()> task;
			if (!global_work_queue.wait_and_pop(task)) break;
				task();
		}
	}
	public:
	thread_pool()
		: done(false)
	{
		int const THREAD_COUNT = std::thread::hardware_concurrency();
		for (int i{}; i < THREAD_COUNT; ++i){
			try{
				threads.push_back(std::thread(&thread_pool::worker_thread, this));
			}
			catch(...){
				done = true;
				throw;
			}
		}
	}

	~thread_pool(){
		done = true;
		global_work_queue.set_done_flag();
		for (auto& t : threads){
			if(t.joinable()) t.join();
		}
	}

	void submit(std::function<void()> new_task){
		global_work_queue.push(std::move(new_task));
	}

};


#endif
