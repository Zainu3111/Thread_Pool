#ifndef LOCAL_QUEUE_HH
#define LOCAL_QUEUE_HH

#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include <condition_variable>
#include <atomic>

// Single Write Multiple Reader lock-free
// data structure specifically to be used 
// as thread-local deque.
template <typename T>
class local_thread_deque{
	private:
		static constexpr size_t CAPACITY = 1024;
		static constexpr size_t MASK = CAPACITY - 1;


		std::atomic<size_t> bottom;
		std::atomic<size_t> top;
		std::vector<std::atomic<T*>> deque{CAPACITY};

	public:
		local_thread_deque(){
			bottom.store(0, std::memory_order_relaxed);
			top.store(0, std::memory_order_relaxed);
		}

		// remove copy assignment operator
		local_thread_deque(const local_thread_deque&) = delete;
		
		// remove assignment operator to reduce complexity
		local_thread_deque operator=(const local_thread_deque) = delete;
		
		// The Owner thread will always push and pop from
		// the top whereas the Thief thread will always pop
		// the bottom. Hence top will only be updated by 
		// Owner thread and bottom will always be updated by 
		// thief thread.
		inline void push(T* new_value){
			size_t cur_top = top.load(std::memory_order_relaxed);

			size_t index = cur_top % CAPACITY;
			
			deque.at(index).store(new_value, std::memory_order_relaxed);

			top.store(cur_top + 1, std::memory_order_release);
		}
		
		// provide a bool if value refrence provided for value else 
		// return a shared pointer to the value;
		// owner always pops from the top
		bool owner_pop(T& value){
			size_t cur_top = top.load(std::memory_order_relaxed);
			size_t cur_bottom = bottom.load(std::memory_order_acquire);
			
			if (cur_top == cur_bottom) return false;
			
			size_t new_top = cur_top - 1;

			if(!top.compare_exchange_strong(cur_top, new_top,
						std::memory_order_acq_rel,
						std::memory_order_relaxed)){
				return false;
			}

			size_t index = new_top % CAPACITY;

			value = deque[index].load(std::memory_order_relaxed);
			return true;
		}
		inline std::shared_ptr<T> owner_pop(){
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
