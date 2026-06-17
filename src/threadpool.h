#ifndef THREADPOOL_HH
#define THREADPOOL_HH


#include<iostream>
#include<vector>
#include<atomic>
#include "./Thread_Safe_Queue/thread_safe_queue.h"
#include <functional>
#include <thread>
#include<condition_variable>
#include <chrono>
#include <future>
class thread_pool{
	std::atomic_bool done;
	threadsafe_queue<std::packaged_task<int()>> global_work_queue;	
	std::vector<std::thread> threads;

	void worker_thread(){
		while(true){
			std::packaged_task<int()> task;
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

	void submit(std::packaged_task<int()> new_task){
		global_work_queue.push(std::move(new_task));
	}

	void worker_while_waiting(std::future<int>& f1, std::future<int>& f2){
		while(
				f1.wait_for(std::chrono::seconds(0)) != std::future_status::ready ||
				f2.wait_for(std::chrono::seconds(0)) != std::future_status::ready){
			std::packaged_task<int()> task;
			if(global_work_queue.try_pop(task)){
				task();
			}else{
				std::this_thread::yield();
			}
		}
	}

};


#endif
