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
	threadsafe_queue<std::function<void()>> gloabl_work_queue;	
	std::vector<std::thread> threads;
	std::condition_variable cv;

	void worker_thread(){
		while(!done){
			std::function<void()> task;
			gloabl_work_queue.wait_and_pop(task);
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
		cv.notify_all();
		int const THREAD_COUNT = std::thread::hardware_concurrency();
		for (int i{}; i < THREAD_COUNT; ++i){
			if(threads.at(i).joinable()) threads.at(i).join();
		}
	}

	void submit(std::function<void()> new_task){
		gloabl_work_queue.push(new_task);
	}

};


#endif
