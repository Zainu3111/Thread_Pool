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
		inline bool push(T* new_value){
			size_t cur_top = top.load(std::memory_order_relaxed);
			size_t cur_bottom = bottom.load(std::memory_order_acquire);

			if (cur_top - cur_bottom > 1024) return false;

			size_t index = cur_top % CAPACITY;
			
			deque.at(index).store(new_value, std::memory_order_relaxed);

			top.store(cur_top + 1, std::memory_order_release);
			return true;
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

		// A Thief will always pop from the bottom. First we will
		// check if there are enough tasks in the local queue and
		// only pop in case we can. The deque must hold atleast
		// 2 local task for it to be eligible to release tasks.
		bool thief_pop(T& value){
			size_t cur_top = top.load(std::memory_order_acquire);
			size_t cur_bottom = bottom.load(std::memory_order_acquire);
			
			if (cur_top == cur_bottom) return false;
			size_t index = cur_bottom % CAPACITY;
			size_t newBottom = cur_bottom + 1;

			auto speculative_read = deque.at(index).load(std::memory_order_relaxed);

			if (bottom.compare_exchange_strong(cur_bottom, newBottom,
						std::memory_order_acq_rel, std::memory_order_relaxed)){
				value = speculative_read;
				return true;
			}
			return false;
		}


		bool empty(){
			size_t cur_top = top.load(std::memory_order_relaxed);
			size_t cur_bottom = bottom.load(std::memory_order_acquire);
			return cur_top == cur_bottom;
		}
		
};
#endif
