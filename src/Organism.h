#pragma once

#include "engine/SimulationObject.h"
#include "engine/SimulationArea.h"
#include "Config.h"
#include "genetics/Genotype.h"
#include "genetics/Phenotype.h"
#include "SensoryData.h"
#include "Planet.h"
#include "ConsumableResourcePool.h"
#include <random>
#include <vector>
#include <atomic>
#include <type_traits>
#include <SFML/System.hpp>

namespace GeneticSimulation
{
	// A living organism that is part of a population
	class Organism : public SimulationObject
	{
	public:

		// constructor
		Organism(unsigned int index, SimulationArea& area, const Config& config);

		// copy constructor (to allow storing in SimulationObjectPool vector)
		Organism(const Organism& rhs);

		// reset and initialize as fresh organism
		void init(sf::Vector2f pos, const Config& config, std::default_random_engine& rng);

		// reset and initialize based on two parent organisms
		void init_from(const Organism& parent1, const Organism& parent2,
			const Config& config, std::default_random_engine& rng);

		// reset and initialize based on single parent organism
		void init_from(const Organism& parent, const Config& config, std::default_random_engine& rng);

		// interact with another organism if close enough
		void interact_with(Organism& other, std::default_random_engine& rng);

		// set physical integrity and heading to best temperature based on surrounding temperature
		void react_to_temperature(const Planet& planet, unsigned int time);

		// increase nutrition (atomic)
		void nourish(unsigned int amount);

		// increase hydration (atomic)
		void hydrate(unsigned int amount);

		// update phenotype after gene transfer
		void update_phenotype();

		// update fitness and existence status
		bool update_fitness();

		// determine distance and heading to closest food item
		void search_for_food(const ConsumableResourcePool& food);

		// determine distance and heading to closest water item
		void search_for_water(const ConsumableResourcePool& water);

		// set heading (velocity) based on sensory data
		void think();

		// update position
		void move();

		// update graphical sprite
		void update_sprite(unsigned int fps);

		// get index
		unsigned int get_index() const;

		// get fitness
		float get_fitness() const;

		// get age
		unsigned int get_age() const;

		// manually set collision status
		void set_collision(unsigned int i);

		// function template for checking if an object is within area of influence
		template<class T>
		bool check_in_range(const T& item, bool center = false) const
		{
			// ensure object class is derived from SimulationObject
			static_assert(std::is_base_of<SimulationObject, T>::value,
				"T must be derived from SimulationObject");
			// get positions and size
			auto item_pos = item.get_position();
			auto pos = get_position();
			auto size = get_size();
			// calculate distance to item
			float d_x, d_y, d_2, range;
			d_x = pos.x - item_pos.x;
			d_y = pos.y - item_pos.y;
			d_2 = d_x * d_x + d_y * d_y;
			// calculate range
			range = size + (center ? 0 : 1) * item.get_size();
			// check whether item is in range
			return d_2 < range * range;
		}

	private:

		// reset any properties not overwritten each time step
		void reset();

		// calculate color based on fitness
		sf::Color calculate_color();

		// calculate outline color
		sf::Color calculate_outline_color(float effect_len);

		// get heading to nearest resource item
		float get_heading_to_nearest_resource(const ConsumableResourcePool& pool);

		// index in population
		const unsigned int index;
		// collection of genetic information
		Genotype genotype;
		// physical traits coded for in genotype
		Phenotype phenotype;
		// data from external and internal senses
		SensoryData sensory_data;
		// age
		unsigned int age;
		// nutrition status
		std::atomic<int> nutrition;
		// hydration status
		std::atomic<int> hydration;
		// physical integrity based on temperature
		int integrity;
		// overall fitness
		float fitness;
		// collision record
		std::vector<uint8_t> collisions;
		// whether gene transfer has occurred
		bool genes_transferred;
		// time gene transfer graphical effect has been active
		int transfer_effect_time;
	};
}