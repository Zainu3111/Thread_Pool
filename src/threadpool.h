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
	public:
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

	void worker_thread(){
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
	thread_pool()
		: done(false), joiner(threads)
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
	}

	template <typename Function_type>
	void submit(Function_type f){
		work_queue.enque(std::function<void()> (f));
	}

};

void run(){
	thread_pool pool;
	std::cout << "Testing threadpool" << std::endl;
	for (int i{}; i < 100; ++i){
		pool.submit([=]{
				std::cout << i << " printed by thread - " << std::this_thread::get_id() << std::endl;
				});
	}
}

#endif
