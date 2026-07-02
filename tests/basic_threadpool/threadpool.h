#ifndef THREADPOOL_HH
#define THREADPOOL_HH


#include<iostream>
#include<vector>
#include<atomic>
#include "../../src/thread_safe_queue.h"
#include <functional>
#include <thread>
#include<condition_variable>
#include <chrono>
#include <future>

#include <cstddef>


class thread_pool{

	
	// done used as atomic bool to make sure when updating done
	// atomically and visible to all threads.
	std::atomic_bool done;

	// Keeps a number in memory. Should be quicker to load from memory
	// than to execute a function every single time we need it. 
	// Furthermore, we can just initialize queues with a default size 
	// rather than reserving extra space later.
	const size_t THREAD_COUNT = std::thread::hardware_concurrency();

	// gloabl work queue used a mutex based lock since majority of
	// the work will be pushed to thread_local_queue by each thread
	// hence no point in using a lock-free queue and introducing
	// live races.
	threadsafe_queue<std::function<void()>> global_work_queue;

	
	// Using a vector for threads since we cannot be sure of the
	// number of threads being used as we use hardware_concurrency
	// function to add init threads at runtime.
	std::vector<std::thread> threads;

	void worker_thread(){

		// Since we are essentially in a spinning lock and done only gets 
		// updated once, having other more rigid memory_models adds unnecessary
		// synchronization.
		while(!done.load(std::memory_order_relaxed) || !global_work_queue.empty()){
			
			std::function<void()> task;
			// Check to see if there is work in global_work_queue. We avoid 
			// stealing first in order to avoid unnecessary synchronization.
			if (global_work_queue.try_pop(task)){
				task();
				continue;
			}
			std::this_thread::yield();

		}
	}
	public:
	thread_pool()
		: done(false)
	{
		threads.reserve(THREAD_COUNT);
		
		// Init threads -> should not cause seg faults now.
		for (int i{}; i < THREAD_COUNT; ++i){
			try{
				threads.push_back(
						std::thread(&thread_pool::worker_thread, this)
						);
			}
			catch(...){
				done.store(true, std::memory_order_relaxed);
				throw;
			}
		}
	}

	~thread_pool(){
		done = true;
		global_work_queue.set_done_flag();
		for (int i{}; i < THREAD_COUNT; ++i){
			if(threads[i].joinable()) threads[i].join();
		}
	}

	template<typename F>
	auto submit(F f) -> std::future<decltype(f())> {
		using Result_Type = decltype(f());
		auto task_ptr = std::make_shared<std::packaged_task<Result_Type()>>(std::move(f));
		std::future<Result_Type> raw_future = task_ptr->get_future();
		
		global_work_queue.push([task_ptr](){
			(*task_ptr)();
			});
		return raw_future;
	}

	// Worker while waiting will work on its own tasks.
	template <typename R>
	void worker_while_waiting(std::future<R>& f1, std::future<R>& f2){
		while(
				f1.wait_for(std::chrono::seconds(0)) != std::future_status::ready ||
				f2.wait_for(std::chrono::seconds(0)) != std::future_status::ready){

			std::function<void()> task;
			// Attempt to work on global queue.
			if (global_work_queue.try_pop(task)){
				task();
				continue;
			}			
			// Yield in case none are satisfied.
			std::this_thread::yield();
		}
	}

};


#endif
