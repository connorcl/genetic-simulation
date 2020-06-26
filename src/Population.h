#pragma once

#include "engine/SimulationObjectPool.h"
#include "Organism.h"
#include "Config.h"
#include "engine/SimulationArea.h"
#include "genetics/StandardizeParams.h"
#include <random>

namespace GeneticSimulation
{
	// A population of organisms
	class Population : public SimulationObjectPool<Organism>
	{
	public:

		// constructor
		Population(SimulationArea& area, const Planet& planet,
			ConsumableResourcePool& food, ConsumableResourcePool& water, const Config& config);

		// initialize the population with a number of organisms
		void init_random(unsigned int n, std::default_random_engine& rng);

		// let organisms in given range interact with nearby organisms
		void interact(unsigned int start, unsigned int end, std::default_random_engine& rng);

		// let organisms in given range react to surrounding temperature
		void react_to_temperature(unsigned int start, unsigned int end, unsigned int time);

		// nourish organisms with given range of items in food pool
		void nourish(unsigned int pool_start, unsigned int pool_end, std::default_random_engine& rng);

		// hydrate organisms with given range of items in water pool
		void hydrate(unsigned int pool_start, unsigned int pool_end, std::default_random_engine& rng);

		// let organisms in given range potentially replicate themselves
		void replicate(unsigned int start, unsigned int end, std::default_random_engine& rng);

		// update phenotypes of each organism in given range if necessary
		void update_phenotypes(unsigned int start, unsigned int end);

		// update fitness of each organism in given range
		void update_fitness(unsigned int start, unsigned int end);

		// let organisms in given range determine heading to nearest food
		void search_for_food(unsigned int start, unsigned int end);

		// let organisms in given range determine heading to nearest water
		void search_for_water(unsigned int start, unsigned int end);

		// let organisms in given range decide on action based on sensory data
		void think(unsigned int start, unsigned int end);

		// let organisms in given range move according to decided heading
		void move(unsigned int start, unsigned int end);

		// update sprite of each organism in given range
		void update_sprites(unsigned int start, unsigned int end);

	private:

		// resource pool types
		enum resource_pool_type { food_pool, water_pool };

		// distribute resources in given range of resource pool to organisms
		void distribute_resources(unsigned int pool_start, unsigned int pool_end, 
			resource_pool_type which_pool, std::default_random_engine& rng);
		
		// reference to area in which organisms exist
		SimulationArea& area;
		// reference to planet on which organisms exist
		const Planet& planet;
		// reference to food and water pools which organisms consume from
		ConsumableResourcePool& food;
		ConsumableResourcePool& water;
		// reference to simulation configuration options
		const Config& config;
	};
}