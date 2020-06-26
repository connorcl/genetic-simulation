#include "Population.h"
#include "ConsumableResourcePool.h"
#include <random>
#include <algorithm>
#include <SFML/System.hpp>

using std::default_random_engine;
using std::uniform_int_distribution;
using std::uniform_real_distribution;
using std::normal_distribution;
using std::min;
using std::max;

using namespace GeneticSimulation;

// constructor
GeneticSimulation::Population::Population(SimulationArea& area, const Planet& planet,
	ConsumableResourcePool& food, ConsumableResourcePool& water, const Config& config) :
	// initialize base class object
	SimulationObjectPool(config.population_size),
	// initialize references to area, planet, food, water and config
	area(area), planet(planet), food(food), water(water), config(config) {}

// initialize the population with a number of organisms
void GeneticSimulation::Population::init_random(unsigned int n, default_random_engine& rng)
{
	// return if already initialized
	if (get_initialized()) return;

	// ensure n is valid
	n = min(get_max_size(), n);
	// create random distributions for position
	auto area_size = area.get_size();
	auto margin = config.population_pos_margin;
	uniform_real_distribution<float> dist_x(margin, area_size.x - margin - 1);
	uniform_real_distribution<float> dist_y(margin, area_size.y - margin - 1);

	// initialize pool
	for (unsigned int i = 0; i < get_max_size(); i++) {
		// add a new uninitialized organism
		add_item(i, area, config);
		// either initialize organism or set index as available
		i < n ? at(i).init(sf::Vector2f(dist_x(rng), dist_y(rng)), config, rng) : 
			set_available(i);
	}

	// record initialization
	set_initialized(true);
}

// let organisms in given range interact with nearby organisms
void GeneticSimulation::Population::interact(unsigned int start, unsigned int end, default_random_engine& rng)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		for (unsigned int j = 0; j < get_max_size(); j++) {
			if (i != j) {
				at(i).interact_with(at(j), rng);
			}
		}
	}
}

// let organisms in given range react to surrounding temperature
void GeneticSimulation::Population::react_to_temperature(unsigned int start, unsigned int end, unsigned int time)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		at(i).react_to_temperature(planet, time);
	}
}

// nourish organisms with given range of items in food pool
void GeneticSimulation::Population::nourish(unsigned int pool_start, 
	unsigned int pool_end, default_random_engine& rng)
{
	distribute_resources(pool_start, pool_end, food_pool, rng);
}

// hydrate organisms with given range of items in water pool
void GeneticSimulation::Population::hydrate(unsigned int pool_start, 
	unsigned int pool_end, default_random_engine& rng)
{
	distribute_resources(pool_start, pool_end, water_pool, rng);
}

// let organisms in given range potentially replicate themselves
void GeneticSimulation::Population::replicate(unsigned int start, unsigned int end, default_random_engine& rng)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	// random distribution for deciding whether to replicate
	uniform_real_distribution<float> dist_replicate(0, 1);
	// probability of replication for current organism
	float replication_prob;
	// for each organism
	for (unsigned int i = start; i < end; i++) {
		// if organism exists
		if (at(i).get_exists()) {
			// calculate probability of replication
			replication_prob = at(i).get_age() < 500 ? 0 : at(i).get_fitness() * config.replication_rate;
			// determine whether to replicate
			if (dist_replicate(rng) < replication_prob) {
				// attempt to get available slot and break if population is full
				if (!get_available_slot(
					[&, i](unsigned int slot) {
						// initialize child from parent
						at(slot).init_from(at(i), config, rng);
						// set parent and child as colliding
						at(i).set_collision(slot);
						at(slot).set_collision(i);
					}
				)) break;
			}
		}
	}
}

// update phenotypes of each organism in given range if necessary
void GeneticSimulation::Population::update_phenotypes(unsigned int start, unsigned int end)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		at(i).update_phenotype();
	}
}

// update fitness of each organism in given range
void Population::update_fitness(unsigned int start, unsigned int end)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		// if not already dead but died this time step
		if (at(i).get_exists() && !at(i).update_fitness()) {
			// add index to available slots
			set_available(i);
		}
	}
}

// let organisms in given range determine heading to nearest food
void GeneticSimulation::Population::search_for_food(unsigned int start, unsigned int end)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		at(i).search_for_food(food);
	}
}

// let organisms in given range determine heading to nearest water
void GeneticSimulation::Population::search_for_water(unsigned int start, unsigned int end)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		at(i).search_for_water(water);
	}
}

// let organisms in given range decide on action based on sensory data
void GeneticSimulation::Population::think(unsigned int start, unsigned int end)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		at(i).think();
	}
}

// let organisms in given range move according to decided heading
void GeneticSimulation::Population::move(unsigned int start, unsigned int end)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		at(i).move();
	}
}

// update sprite of each organism in given range
void GeneticSimulation::Population::update_sprites(unsigned int start, unsigned int end)
{
	if (!get_initialized()) return;

	end = min(get_max_size(), end);

	for (unsigned int i = start; i < end; i++) {
		at(i).update_sprite(config.standard_framerate);
	}
}

// distribute resources in given range of resource pool to organisms
void GeneticSimulation::Population::distribute_resources(unsigned int pool_start, 
	unsigned int pool_end, resource_pool_type which_pool, default_random_engine& rng)
{
	// get reference to relevant pool
	auto& pool = (which_pool == food_pool ? food : water);
	// ensure end is valid
	pool_end = min(pool.get_max_size(), pool_end);
	// for each item
	for (unsigned int i = pool_start; i < pool_end; i++) {
		// if item exists
		if (pool[i].get_exists()) {
			// for each organism
			for (unsigned int j = 0; j < get_max_size(); j++) {
				// if organism is alive and in range to consume item
				if (at(j).get_exists() && at(j).check_in_range(pool[i])) {
					// let organism consume item
					if (which_pool == food_pool) {
						at(j).nourish(pool.consume_and_reset_item(i, rng));
					}
					else {
						at(j).hydrate(pool.consume_and_reset_item(i, rng));
					}
					// break as only one organism may consume item
					break;
				}
			}
		}
	}
}