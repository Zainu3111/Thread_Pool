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
		std::atomic<long> bottom = 0;

		// Top points to the first stealable instance in the queue.
		std::atomic<long> top = 0;

		// We have a vector of size 1024 that stores atomic ptrs
		// as the deque.
		std::atomic<T*> deque[CAPACITY];

		// Reference Count to figure out if we can delete the queue
		std::atomic<int> ref_count{0};

		
		bool no_thief_popping(){
			//TODO add functionality
			return false;
		}


	public:

		local_thread_deque() = default;

		~local_thread_deque(){
		// Preconditions:
		// 					all worker threads HAVE terminated
			T* task;
			while(owner_pop(task)){
				delete task;
			}
		}

		// remove copy assignment operator
		local_thread_deque(const local_thread_deque&) = delete;
		
		// remove assignment operator to reduce complexity
		local_thread_deque& operator=(const local_thread_deque&) = delete;
		
		// The Owner thread will always push and pop from the bottom.
		// It is essentially a stack for the owner
		inline bool push(T* new_value){
			long cur_bottom = bottom.load(std::memory_order_relaxed);
			
			long cur_top = top.load(std::memory_order_acquire);

			auto size = cur_bottom - cur_top;
			if (size >= CAPACITY) return false;

			auto index = static_cast<size_t>(cur_bottom % CAPACITY);
			
			deque[index].store(new_value, std::memory_order_relaxed);

			bottom.store(cur_bottom + 1, std::memory_order_release);
			return true;
		}
		
		// Provide a bool if value refrence provided for value else 
		// return a shared pointer to the value. Owner always pops
		// from the bottom.
		bool owner_pop(T*& value){
			// load bottom from memory. Since only owner changes
			// bottom, there is no need for synchronization.
			long cur_bottom = bottom.load(std::memory_order_relaxed);
			
			// Decrement bottom to show we are attempting poping.
			--cur_bottom;
			bottom.store(cur_bottom, std::memory_order_release);

			// Load top.
			long cur_top = top.load(std::memory_order_acquire);
			
			// check if empty.
			auto size = cur_bottom - cur_top;
			
			// Bottom can be reordered by the compiler or the 
			// architecture, so need a memory fence here.
			std::atomic_thread_fence(std::memory_order_seq_cst);

			if (size < 0){
				bottom.store(cur_top, std::memory_order_release);
				return false;
			}
			// Speculative reading.
			auto index = static_cast<size_t>(cur_bottom % CAPACITY);
			auto speculative_read = deque[index].load(std::memory_order_relaxed);

			if(size > 0){
				value = speculative_read;
				return true;
			}
			
			// In case we only have 1 element.
			if(top.compare_exchange_strong(cur_top, cur_top + 1,
				std::memory_order_acq_rel, std::memory_order_relaxed)
			){
				bottom.store(cur_top + 1, std::memory_order_release);
				value = speculative_read;
				return true;
			}else{
				bottom.store(cur_top + 1, std::memory_order_release);
				return false;
			}
		
		}

		// A Thief will always pop from the top. First we will check
		// if there are enough tasks in the local queue.
		bool thief_pop(T*& value){

			// We load bottom and figure out the size of the arr.
			long cur_bottom = bottom.load(std::memory_order_acquire);

			// We read top
			long cur_top = top.load(std::memory_order_acquire);

			long size = cur_bottom - cur_top;
			
			if (size <= 0) return false;
			
			// Speculatively read from the local queue.
			auto index = static_cast<size_t>(cur_top % CAPACITY);
			auto speculative_read = deque[index].load(std::memory_order_relaxed);

			// Comapare and swap top. If it succeeds then we have
			// successfully stolen the top.
			if (!top.compare_exchange_strong(cur_top, cur_top + 1,
						std::memory_order_acq_rel, std::memory_order_relaxed)){
				return false;
			}
			value = speculative_read;
			return true;
		}


		bool empty(){
			auto cur_top = top.load(std::memory_order_relaxed);
			auto cur_bottom = bottom.load(std::memory_order_acquire);
			return cur_top == cur_bottom;
		}
		
};
#endif
