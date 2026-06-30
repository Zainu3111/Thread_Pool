#ifndef THREADPOOL_HH
#define THREADPOOL_HH


#include<iostream>
#include<vector>
#include<atomic>
#include "../Thread_Safe_Queue/thread_safe_queue.h"
#include "../Thread_Safe_Queue/local_thread_safe_queue.h"
#include <functional>
#include <thread>
#include<condition_variable>
#include <chrono>
#include <future>

#include <cstddef>


class thread_pool{

	// With static, compiler has to make sure it is only initialized
	// once, which leads to a hidden branch which though predictable
	// adds extra instructions. So, I decided to have a vector of
	// local_queues whichc should only add some loading instructions.
	inline static thread_local size_t tl_worker_id = static_cast<size_t>(-1);
	
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

	// A vector of all the local queues to allow for stealing.
	std::vector<local_thread_deque<std::function<void()>>*> queue_set;
	
	// Using a vector for threads since we cannot be sure of the
	// number of threads being used as we use hardware_concurrency
	// function to add init threads at runtime.
	std::vector<std::thread> threads;

	//////////////////////////////////////////////////////////////////
	void worker_thread(size_t threadId){
		tl_worker_id = threadId;

		// Since we are essentially in a spinning lock and done only gets 
		// updated once, having other more rigid memory_models adds unnecessary
		// synchronization.
		while(!done.load(std::memory_order_relaxed)){
			
			std::function<void()>* task_ptr = nullptr;

			// Checking local queue first in case we have pending work before we
			// try to steal or go to global work queue to avoid unnecessary work.
			if (queue_set[tl_worker_id]->owner_pop(task_ptr)){
				(*task_ptr)();
				delete task_ptr;
				continue;
			}

			std::function<void()> task;
			// Check to see if there is work in global_work_queue. We avoid 
			// stealing first in order to avoid unnecessary synchronization.
			if (global_work_queue.try_pop(task)){
				task();
				continue;
			}

			// Attempt to steal work from other threads. Using threads.size or 
			// queue_set.size for checking other queues rather than thread count 
			// in case os throws an exception and is unable to allocate a thread.
			// Better to use the size rather than hard coded numbers.
			bool stole_work = false;
			for(size_t i{}; i < threads.size(); ++i){
				if(i == tl_worker_id) continue;
				if (queue_set[i] && queue_set[i]->thief_pop(task_ptr)){
					stole_work = true;
					break;
				}
			}
			if (stole_work){
				(*task_ptr)();
				delete task_ptr;
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
		queue_set.reserve(THREAD_COUNT);
		
		// Init local queue before we init the thread since we 
		// want to access the local queue and might access before
		// it is init -> random seg faults.
		for (int x{}; x < THREAD_COUNT; ++x){
			queue_set.push_back(new local_thread_deque<std::function<void()>>);
		}
		
		// Init threads -> should not cause seg faults now.
		for (int i{}; i < THREAD_COUNT; ++i){
			try{
				threads.push_back(
						std::thread(&thread_pool::worker_thread, this, i)
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
			delete queue_set[i];
		}
	}

	template<typename F>
	auto submit(F f) -> std::future<decltype(f())> {
		using Result_Type = decltype(f());
		auto task_ptr = std::make_shared<std::packaged_task<Result_Type()>>(std::move(f));
		std::future<Result_Type> raw_future = task_ptr->get_future();
		
		bool push_success{false};
		if (tl_worker_id != static_cast<size_t>(-1)){
			auto* heap_ptr = new std::function<void()>(
					[task_ptr](){ (*task_ptr)(); }
					);
			if(queue_set[tl_worker_id]->push(heap_ptr)){
				push_success = true;
			}else{
				delete heap_ptr;
			}
		}
		if (!push_success){
			global_work_queue.push([task_ptr](){
						(*task_ptr)();
					});
		}
		return raw_future;
	}

	// Worker while waiting will work on its own tasks.
	template <typename R>
	void worker_while_waiting(std::future<R>& f1, std::future<R>& f2){
		while(
				f1.wait_for(std::chrono::seconds(0)) != std::future_status::ready ||
				f2.wait_for(std::chrono::seconds(0)) != std::future_status::ready){
			std::function<void()>* task_ptr;
			// Attempt to work from own queue.
			bool is_worker = (tl_worker_id != static_cast<size_t>(-1));

			if(is_worker && queue_set[tl_worker_id]->owner_pop(task_ptr)){
				(*task_ptr)();
				delete task_ptr;
				continue;
			}

			std::function<void()> task;
			// Attempt to work on global queue.
			if (global_work_queue.try_pop(task)){
				task();
				continue;
			}

			// Attempt to steal work from other threads.
			bool stole_work = false;
			for(size_t i{}; i < queue_set.size(); ++i){
				if(i == tl_worker_id) continue;
				if (queue_set[i]->thief_pop(task_ptr)){
					stole_work = true;
					break;
				}
			}
			if (stole_work){
				(*task_ptr)();
				delete task_ptr;
				continue;
			}
			
			// Yield in case none are satisfied.
			std::this_thread::yield();
		}
	}

};


#endif
