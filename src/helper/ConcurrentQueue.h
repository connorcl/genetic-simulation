#pragma once

#include <queue>
#include <mutex>

namespace GeneticSimulation
{
	// A simple extension of a queue allowing thread-safe pushes and pops
	template<typename T>
	class ConcurrentQueue
	{
	public:

		// thread-safe push
		void safe_push(T item) {
			// lock mutex
			std::scoped_lock lock(mx);
			// push item
			q.push(item);
		}

		// thread-safe pop which pops an item off if available and applies the
		// given callable object to the popped item
		template<class F>
		bool safe_pop(F pop_callback) {
			// lock mutex
			std::unique_lock lock(mx);
			// return false if queue is empty
			if (q.empty()) {
				return false;
			}
			// pop and apply callable object otherwise
			else {
				// get item
				T item = q.front();
				// pop item
				q.pop();
				// unlock mutex
				lock.unlock();
				// apply given callable object to popped item
				pop_callback(item);
				// return true to indicate pop was successful
				return true;
			}
		}

		// provide access to the underlying queue
		std::queue<T>& get_queue() {
			return q;
		}

	private:

		// mutex protecting queue
		std::mutex mx;
		// queue
		std::queue<T> q;
	};
}