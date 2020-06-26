#pragma once

#include "engine/SimulationObjectPool.h"
#include "ConsumableResource.h"
#include "engine/SimulationArea.h"
#include <random>

namespace GeneticSimulation
{
	// A pool of consumable resources
	class ConsumableResourcePool : public SimulationObjectPool<ConsumableResource>
	{
	public:

		// constructor
		ConsumableResourcePool(unsigned int max_size, unsigned int max_val, 
			sf::Color item_color, float margin, SimulationArea& area);

		// set up the pool and randomly initialize a number of items
		void init_random(unsigned int n, std::default_random_engine& rng);

		// consume an item and reset its position and value
		unsigned int consume_and_reset_item(unsigned int i, std::default_random_engine& rng);

	private:

		// reset an item
		void reset_item(unsigned int i, std::default_random_engine& rng);

		// maximum energy value
		const unsigned int max_val;
		// margin for generating resource positions
		const float margin;
		// color of items
		const sf::Color item_color;
		// reference to area in which resources exist
		SimulationArea& area;
	};
}