#ifndef THREADPOOL_HH
#define THREADPOOL_HH


#include<iostream>
#include<vector>
#include<atomic>
#include "../../src/Mutex_Queue/mutex_queue.h"
#include <functional>
#include<thread>
#include<future>

class thread_pool{
	std::atomic_bool done;
	mutex_queue<std::function<void()>> work_queue;	
	std::vector<std::thread> threads;
	std::condition_variable cv;
	std::mutex m;

	void worker_thread(){
		while(true){
			std::function<void()> task;
			
			{
				std::unique_lock<std::mutex> lock(m);
				cv.wait(lock, [&](){
					return done ||! work_queue.is_empty();
						});
				if(done && work_queue.is_empty()){
					return;
				}
				task = (work_queue.deque()).value();
			}
			task();
		}


		while(!done || work_queue.size() > 0){
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
	thread_pool()
		: done(false)
	{
		int const thread_count = std::thread::hardware_concurrency();
		for (int i{}; i < thread_count; ++i){
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
		for(auto& t : threads){
			if(t.joinable()){
				t.join();
			}
		}
	}


	template <typename Function_type>
	void submit(Function_type f){
		work_queue.enque(f);
		cv.notify_one();
	}

};


#endif
