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
	mutex_queue<std::function<void()>> global_work_queue;	
	std::vector<std::thread> threads;
	std::condition_variable cv;
	std::mutex m;
	typedef std::queue<std::function<void()>> local_queue_type;
	static thread_local std::unique_ptr<local_queue_type> local_work_queue;

	void worker_thread(){
		local_work_queue.reset(new local_queue_type);
		while(!done){
			std::function<void()> task;
			{
				if(local_work_queue && !local_work_queue->empty()){
					task = local_work_queue->front();
					local_work_queue->pop();
				}else if(global_work_queue.is_empty()){
					std::unique_lock<std::mutex> lock(m);
					cv.wait(lock, [&](){
						return done || !local_work_queue->empty() || !global_work_queue.is_empty();
						});
					continue;
				}else{
				auto cur = (global_work_queue.deque());
				if(cur.has_value()){
					task = cur.value();
				}else{
					continue;
				}
				}
			}
			task();
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
		if(local_work_queue){
			local_work_queue->push(f);
		}else{
			global_work_queue.enque(f);
		}
		cv.notify_one();
	}


};


#endif
