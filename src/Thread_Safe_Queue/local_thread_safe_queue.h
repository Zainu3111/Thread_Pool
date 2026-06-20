#ifndef QUEUE_HH
#define QUEUE_HH

#include <iostream>
#include <vector>
#include <optional>
#include <queue>
#include <memory>
#include <condition_variable>


// Single Write Multiple Reader lock-free
// data structure specifically to be used 
// as thread-local queue.
template <typename T>
class local_thread_deque{
	private:
		mutable std::mutex mut;
		std::condition_variable data_cond;
		std::queue<T> data_queue;
		std::atomic_bool DONE = false;

	public:
		local_thread_deque() = default;
		local_thread_deque(const local_thread_deque&);
		
		// remove assignment operator to reduce complexity
		local_thread_deque operator=(const local_thread_deque) = delete;
		
		inline void push(T new_value){
			std::lock_guard<std::mutex> lock(mut);
			data_queue.push(std::move(new_value));
			data_cond.notify_one();
		}
		
		// provide a bool if value refrence provided for value else 
		// return a shared pointer to the value;
		bool try_pop_left(T& value){
			std::lock_guard<std::mutex> lock(mut);
			if(data_queue.empty()){
				return false;
			}
			value = std::move(data_queue.front());
			data_queue.pop();
			return true;
		}
		inline std::shared_ptr<T> try_pop_left(){
			std::lock_guard<std::mutex> lock(mut);
			if(data_queue.empty()){
				return nullptr;
			}
			std::shared_ptr<T> ptr = std::make_shared<T>(std::move(data_queue.front()));
			data_queue.pop();
			return ptr;
		}


		// similarly for wait and pop
		inline bool wait_and_pop_left(T& value){
			std::unique_lock<std::mutex> lock(mut);
			data_cond.wait(lock, [this](){
				return !data_queue.empty() || DONE;	
			});
			if(DONE && data_queue.empty()){
				return false;
			}
			value = std::move(data_queue.front());
			data_queue.pop();
			return true;
		}

		inline std::shared_ptr<T> wait_ant_pop_left(){
			std::unique_lock<std::mutex> lock(mut);
			data_cond.wait(lock, [this](){
				return !data_queue.empty() || DONE;	
			});
			if(DONE && data_queue.empty()){
				return nullptr;
			}
			std::shared_ptr<T> value = std::make_shared<T>(std::move(data_queue.front()));
			data_queue.pop();
			return value;
		}

		bool empty(){
			std::lock_guard<std::mutex> lock(mut);
			return data_queue.empty();
		}
		
		void set_done_flag(){
			DONE = true;
			data_cond.notify_all();
		}
};
#endif
