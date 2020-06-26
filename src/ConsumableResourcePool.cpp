#include "ConsumableResourcePool.h"
#include <random>

using std::default_random_engine;
using std::uniform_int_distribution;

using namespace GeneticSimulation;

// constructor
GeneticSimulation::ConsumableResourcePool::ConsumableResourcePool(unsigned int max_size, 
	unsigned int max_val, sf::Color item_color, float margin, SimulationArea& area) :
	// initialize base class object
	SimulationObjectPool(max_size), 
	// initialize member variables
	max_val(max_val), margin(margin),
	item_color(item_color), area(area) {}

// randomly initialize a number of items
void GeneticSimulation::ConsumableResourcePool::init_random(unsigned int n, default_random_engine& rng)
{
	// return if pool is already initialized
	if (get_initialized()) return;

	// fill pool with uninitialized items and initialize first n items
	for (unsigned int i = 0; i < get_max_size(); i++) {
		add_item(item_color, area);
		// randomly initialize first n items
		if (i < n) {
			reset_item(i, rng);
		}
		// add indices of uninitialized items to available slots queue
		else {
			set_available(i);
		}
	}

	// record intialization
	set_initialized(true);
}

// consume an item and reset its position and value
unsigned int GeneticSimulation::ConsumableResourcePool::consume_and_reset_item(unsigned int i, default_random_engine& rng)
{
	// consume item and save value
	auto value = at(i).consume();
	// reset item
	reset_item(i, rng);
	// return item value
	return value;
}

// reset an item
void GeneticSimulation::ConsumableResourcePool::reset_item(unsigned int i, default_random_engine& rng)
{
	// get area bounds
	auto area_size = area.get_size();

	// create distributions for position and value of item
	uniform_int_distribution<int> dist_x(margin, area_size.x - margin - 1);
	uniform_int_distribution<int> dist_y(margin, area_size.y - margin - 1);
	uniform_int_distribution<int> dist_val(10000, max_val);

	// initialize item
	at(i).init(dist_val(rng), max_val, sf::Vector2f(dist_x(rng), dist_y(rng)));
}