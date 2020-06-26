#pragma once

#include "helper/platform.h"
#include "Config.h"
#include <vector>

namespace GeneticSimulation
{
	// precomputes and stores planetary surface temperature
	class Planet
	{
	public:

		// default constructor
		Planet();

		// precompute temperatures
		void precompute_temperatures(const Config& config, bool benchmark = false);

		// get temperature from lookup table
		float get_temperature(unsigned int y, unsigned int t) const;

	private:

		// precompute temperatures on the CPU
		void precompute_temperatures_cpu(unsigned int worker_threads, const Config& config);

		// precompute temperatures on the CPU for the given timestep range
		void precompute_temperatures_for_timestep_range_cpu(unsigned int start_t, 
			unsigned int end_t, const Config& config);

#ifdef GPU_SUPPORT
		// precompute temperatures on the GPU
		void precompute_temperatures_gpu(const Config& config);
#endif

		// benchmark precomputation on the CPU
		void benchmark_temperature_computation_cpu(unsigned int worker_threads, const Config& config);

#ifdef GPU_SUPPORT
		// benchmark precomputation on the GPU
		void benchmark_temperature_computation_gpu(const Config& config);
#endif

		// whether temperatures have been precomputed
		bool initialized;
		// lookup table for temperature
		std::vector<float> temperatures;
		// number of timesteps in orbital period, used 
		// for calculating indexes in lookup table
		unsigned int timesteps;
	};
}