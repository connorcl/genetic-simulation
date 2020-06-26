#pragma once

#include "SimulationObject.h"
#include "../helper/ConcurrentQueue.h"
#include <type_traits>
#include <vector>

namespace GeneticSimulation
{
	// class template for a pool of simulation objects
	template<class T>
	class SimulationObjectPool
	{
		// ensure T is derived class of SimulationObject
		static_assert(std::is_base_of<SimulationObject, T>::value, 
			"T must be derived from SimulationObject");

	public:

		// constructor
		explicit SimulationObjectPool(unsigned int max_size) :
			initialized(false), max_size(max_size) {}

		// element access operators and functions
		T& operator[](unsigned int i) { return pool[i]; }
		const T& operator[](unsigned int i) const { return pool[i]; }
		T& at(unsigned int i) { return pool[i]; }
		const T& at(unsigned int i) const { return pool[i]; }

		// getters for max size and initialization status
		unsigned int get_max_size() const { return max_size; }
		bool get_initialized() const { return initialized; }

		// draw pool items
		void draw() {
			for (auto& i : pool) {
				i.draw();
			}
		}

	protected:

		// emplace a new object
		template<class... Args>
		void add_item(Args&&... args) {
			pool.emplace_back(args...);
		}

		// set initialization status
		void set_initialized(bool status) { initialized = status; }

		// get next available slot index and perform an operation on it
		template<class F>
		bool get_available_slot(F pop_callback) {
			return available_slots.safe_pop(pop_callback);
		}

		// set a slot index as available
		void set_available(unsigned int i) { 
			available_slots.safe_push(i);
		}

	private:

		// whether pool has been initialized
		bool initialized;
		// maximum size of the pool
		const unsigned int max_size;
		// pool of simulation objects
		std::vector<T> pool;
		// concurrent queue for keeping track of available/unallocated slots in the pool
		ConcurrentQueue<unsigned int> available_slots;
	};
}