#ifndef LOCAL_QUEUE_HH
#define LOCAL_QUEUE_HH

#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include <condition_variable>
#include <atomic>


// Implementation of Chase-Lev queue as proposed by their paper 
// "Dynamic Circular Work-Stealing Deque" from 2005.

// Single Write Multiple Reader lock-free data structure
// specifically to be used as thread-local deque.
template <typename T>
class local_thread_deque{
	private:
		// CAPACITY will be used for the wrap around effect for
		// the queue. Current implemtation does not use the
		// dynamic array affect.
		static constexpr size_t CAPACITY = 1024;

		// Bottom points to the first empty slot for the owner.
		std::atomic<size_t> bottom = 0;

		// Top points to the first stealable instance in the queue.
		std::atomic<size_t> top = 0;

		// We have a vector of size 1024 that stores atomic ptrs
		// as the deque.
		std::atomic<T*> deque[CAPACITY];

	public:

		local_thread_deque(){
			bottom.store(0, std::memory_order_relaxed);
			top.store(0, std::memory_order_relaxed);
		}

		// remove copy assignment operator
		local_thread_deque(const local_thread_deque&) = delete;
		
		// remove assignment operator to reduce complexity
		local_thread_deque operator=(const local_thread_deque) = delete;
		
		// The Owner thread will always push and pop from the bottom.
		// It is essentially a stack for the owner
		inline bool push(T* new_value){
			size_t cur_bottom = top.load(std::memory_order_relaxed);
			size_t cur_top = bottom.load(std::memory_order_acquire);

			if (cur_bottom - cur_top >= 1024) return false;

			size_t index = cur_bottom % CAPACITY;
			
			deque.at(index).store(new_value, std::memory_order_relaxed);

			bottom.store(cur_bottom + 1, std::memory_order_release);
			return true;
		}
		
		// Provide a bool if value refrence provided for value else 
		// return a shared pointer to the value. Owner always pops
		// from the bottom.
		bool owner_pop(T& value){
			// load bottom from memory. Since only owner changes
			// bottom, there is no need for synchronization.
			size_t cur_bottom = bottom.load(std::memory_order_relaxed);

			// Decrement bottom to show we are attempting poping.
			--cur_bottom;
			bottom.store(cur_bottom, std::memory_order_release);

			// Speculative reading.
			size_t index = cur_bottom % CAPACITY;
			auto temp = deque[index].load(std::memory_order_relaxed);

			// Load top.
			size_t cur_top = top.load(std::memory_order_acquire);

			// check if empty.
			int size = cur_bottom - cur_top;
			if (size < 0){
				bottom.store(cur_top, std::memory_order_release);
				return false;
			}
			if(size == 0){ 
				if(top.compare_exchange_strong(cur_top, cur_top + 1,
					std::memory_order_acq_rel, std::memory_order_relaxed)
				){
					bottom.store(cur_top + 1, std::memory_order_release);
					value = *temp;
					return true;
				}else{
					bottom.store(cur_top + 1, std::memory_order_release);
					return false;
				}
			}
			
			value = *temp;
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
				value = *speculative_read;
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
