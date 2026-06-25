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
	inline static thread_local local_thread_deque<std::function<void()>>* thread_local_queue = nullptr;
	inline static thread_local size_t tl_worker_id = static_cast<size_t>(-1);
	std::atomic_bool done;
	threadsafe_queue<std::function<void()>> global_work_queue;	
	std::vector<std::thread> threads;

	std::vector<local_thread_deque<std::function<void()>>> local_work_queues;

	void worker_thread(size_t threadId){
		tl_worker_id = threadId;

		while(!done.load(std::memory_order_relaxed)){
			std::function<void()> task;
			// Check to see if there is work available in
			// local_qork_queue. If no work then move to
			// global_work_queue.
			if (local_work_queues[threadId].owner_pop(task)){
				task();
				continue;
			}
			// Check to see if there is work in
			// global_work_queue. if there is work in it,
			// pop and run task. else try to steal work 
			// from other threads.
			if (global_work_queue.try_pop(task)){
				task();
				continue;
			}
			// Attempt to steal work from other threads.
			bool stole_work = false;
			for(size_t i{}; i < local_work_queues.size(); ++i){
				if(i == threadId) continue;
				if (local_work_queues[i].thief_pop(task)){
					stole_work = true;
					break;
				}
			}
			if (stole_work){
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
		int const THREAD_COUNT = std::thread::hardware_concurrency();
		for (int i{}; i < THREAD_COUNT; ++i){
			try{
				threads.push_back(std::thread(&thread_pool::worker_thread, this, i));
				local_work_queues.push_back(local_thread_deque<std::function<void()>>{});
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
		for (auto& t : threads){
			if(t.joinable()) t.join();
		}
	}

	template<typename F>
	auto submit(F f) -> std::future<decltype(f())> {
		using Result_Type = decltype(f());
		auto task_ptr = std::make_shared<std::packaged_task<Result_Type()>>(std::move(f));
		std::future<Result_Type> raw_future = task_ptr->get_future();
		
		
		if (tl_worker_id != static_cast<size_t>(-1)){
			auto wrapper = std::make_shared<std::function<void()>>
				([task_ptr](){ (*task_ptr)(); });
			std::function<void()>* raw_ptr = wrapper.get();
			local_work_queues[tl_worker_id].push(raw_ptr);
		}else{
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
			std::function<void()> task;
			// Attempt to work from own queue.
			bool is_worker = (tl_worker_id != static_cast<size_t>(-1));

			if(is_worker && local_work_queues[tl_worker_id].owner_pop(task)){
				task();
				continue;
			}

			// Attempt to work on global queue.
			if (global_work_queue.try_pop(task)){
				task();
				continue;
			}

			// Attempt to steal work from other threads.
			bool stole_work = false;
			for(size_t i{}; i < local_work_queues.size(); ++i){
				if(i == tl_worker_id) continue;
				if (local_work_queues[i].thief_pop(task)){
					stole_work = true;
					break;
				}
			}
			if (stole_work){
				task();
				continue;
			}
			
			// Yield in case none are satisfied.
			std::this_thread::yield();
		}
	}

};


#endif
