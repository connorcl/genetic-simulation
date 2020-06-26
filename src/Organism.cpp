#include "Organism.h"
#include "genetics/StandardizeParams.h"
#include "helper/numbers.h"
#include "helper/color.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <limits>

using std::default_random_engine;
using std::uniform_real_distribution;
using std::min;
using std::max;
using std::numeric_limits;

using namespace GeneticSimulation;

// constructor
GeneticSimulation::Organism::Organism(unsigned int index,
	SimulationArea& area, const Config& config) :
	// initialize base class object and unique index
	SimulationObject(area), index(index),
	// initialize genotype
	genotype(7, config.behaviour_net_layer_1_units, config.behaviour_net_layer_2_units, 2),
	// initialize phenotype with population-wide parameters
	phenotype(
		StandardizeParams(config.area_of_influence_mean, config.area_of_influence_sigma),
		StandardizeParams(config.speed_mean, config.speed_sigma),
		StandardizeParams(config.health_rate_mean, config.health_rate_sigma),
		StandardizeParams(config.ideal_temp_mean, config.ideal_temp_sigma),
		StandardizeParams(config.temp_range_mean, config.temp_range_sigma)
	),
	// initialize age and fitness components
	age(0), nutrition(one_million), hydration(one_million), integrity(one_million), fitness(1.f),
	// initialize collisions record to size of population
	collisions(config.population_size, 0),
	// initialize gene transfer and gene transfer effect as inactive
	genes_transferred(false), transfer_effect_time(-1)
{
	// set up sprite outline
	set_sprite_outline_thickness(-1.5f);
}

// copy constructor (to allow storing in SimulationObjectPool vector)
GeneticSimulation::Organism::Organism(const Organism& rhs) :
	SimulationObject(rhs),
	index(rhs.index),
	genotype(rhs.genotype),
	phenotype(rhs.phenotype),
	sensory_data(rhs.sensory_data),
	age(rhs.age),
	nutrition(rhs.nutrition.load()),
	hydration(rhs.hydration.load()),
	integrity(rhs.integrity),
	fitness(rhs.fitness),
	collisions(rhs.collisions),
	genes_transferred(rhs.genes_transferred),
	transfer_effect_time(rhs.transfer_effect_time) {}

// reset and initialize as fresh organism
void GeneticSimulation::Organism::init(sf::Vector2f pos, const Config& config, 
	default_random_engine& rng)
{
	// reset necessary status items
	reset();
	// set position
	set_position(pos);
	// initialize random genotype
	genotype.init_random(config.behaviour_net_weight_range, 
		config.behaviour_net_weight_range_bias, rng);
	// calculate traits from genotype
	genotype.express_traits(phenotype);
	// set size based on area of influence
	set_size(phenotype.get_area_of_influence());
	// set existence status
	set_exists(true);
}

// reset and initialize based on two parent organisms
void GeneticSimulation::Organism::init_from(const Organism& parent1, 
	const Organism& parent2, const Config& config, default_random_engine& rng)
{
	// reset necessary status items
	reset();
	// position is average of parents
	set_position((parent1.get_position() + parent2.get_position()) / 2.f);
	// initialize genotype from parents
	genotype.init_from(parent1.genotype, parent2.genotype, 
		config.behaviour_net_mutation_prob, config.behaviour_net_mutation_sigma,
		config.trait_genes_mutation_prob, config.trait_genes_mutation_sigma, rng);
	// calculate traits from genotype
	genotype.express_traits(phenotype);
	// set size based on area of influence
	set_size(phenotype.get_area_of_influence());
	// set existence status
	set_exists(true);
}

// reset and initialize based on single parent organism
void GeneticSimulation::Organism::init_from(const Organism& parent,
	const Config& config, default_random_engine& rng)
{
	// reset necessary status items
	reset();
	// set position from parent
	set_position(parent.get_position());
	// initialize genotype from parent
	genotype.init_from(parent.genotype, 
		config.behaviour_net_mutation_prob, config.behaviour_net_mutation_sigma, 
		config.trait_genes_mutation_prob, config.trait_genes_mutation_sigma, rng);
	// calculate traits from genotype
	genotype.express_traits(phenotype);
	// set size based on area of influence
	set_size(phenotype.get_area_of_influence());
	// set existence status
	set_exists(true);
}

// interact with another organism if close enough
void GeneticSimulation::Organism::interact_with(Organism& other,
	default_random_engine& rng)
{
	// return if not alive
	if (!get_exists()) return;
	// clear corresponding collision status and return if other is not alive
	if (!other.get_exists()) { collisions[other.index] = 0; return; }

	// if collision occurred and was not previously ongoing, and other is older than 250
	auto collision = check_in_range(other, true);
	if (collision && !collisions[other.index] && other.age > 250) {
		// determine whether to transfer genes
		uniform_real_distribution<float> dist_transfer(0.f, 1.f);
		auto chance_of_transfer = (fitness * 0.35f + other.fitness * 0.65f) / 10.f;
		if (dist_transfer(rng) < chance_of_transfer) {
			// determine how much of other genotype to transfer
			auto weighting = (((other.fitness - fitness) / 2.f) + 0.5f) / 5.f;
			// transfer information
			genotype.transfer_from(other.genotype, weighting);
			// record transfer
			genes_transferred = true;
			transfer_effect_time = 0;
		}
	}
	// record collision status
	collisions[other.index] = collision;
}

// set physical integrity and heading to best temperature based on surrounding temperature
void GeneticSimulation::Organism::react_to_temperature(const Planet& planet, unsigned int time)
{
	// return if not alive
	if (!get_exists()) return;

	// get position
	auto position = get_position();
	// get temperature at current position
	auto current_temp = planet.get_temperature(static_cast<unsigned int>(position.y), time);
	// get difference between current temp and ideal temp
	auto temp_d = abs(current_temp - phenotype.get_ideal_temp());
	// calculate impact on integrity
	integrity = (temp_d < phenotype.get_temp_range()) ?
		min(1e6f, integrity + phenotype.get_health_rate() / max(1.f, temp_d)) :
		max(0.f, integrity - temp_d / (120.f / (phenotype.get_health_rate() / 2.f)));
	// update temperature damage in sensory data
	sensory_data.set_temperature_damage(integrity);

	// get temperature north of current position
	auto north_temperature = planet.get_temperature(max(0, static_cast<int>(position.y) - 5), time);
	// get temperature south of current position
	auto south_temperature = planet.get_temperature(min(static_cast<int>(get_area_size().y) - 1,
		static_cast<int>(position.y) + 5), time);
	// calculate which direction is more habitable temperature and set heading
	auto north_d = abs(north_temperature - phenotype.get_ideal_temp());
	auto south_d = abs(south_temperature - phenotype.get_ideal_temp());
	auto temperature_heading = north_d < south_d ? pi / 2 : -pi / 2;
	// set temperature heading in sensory data
	sensory_data.set_temperature_heading(temperature_heading);
}

// increase nutrition (atomic)
void GeneticSimulation::Organism::nourish(unsigned int amount)
{
	nutrition.fetch_add(amount);
}

// increase hydration (atomic)
void GeneticSimulation::Organism::hydrate(unsigned int amount)
{
	hydration.fetch_add(amount);
}

// update phenotype after gene transfer
void GeneticSimulation::Organism::update_phenotype()
{
	if (genes_transferred) {
		// update physical traits
		genotype.express_traits(phenotype);
		// set size based on area of influence
		set_size(phenotype.get_area_of_influence());
		// clear transferred genes flag
		genes_transferred = false;
	}
}

// update fitness and existence status
bool GeneticSimulation::Organism::update_fitness()
{
	// return if not alive
	if (!get_exists()) return false;

	// cap nutrition and hydration and subtract health rate value
	nutrition = min(one_million, nutrition.load());
	nutrition -= phenotype.get_health_rate();
	hydration = min(one_million, hydration.load());
	hydration -= phenotype.get_health_rate();

	// die if any health stat is 0
	if (nutrition <= 0 || hydration <= 0 || integrity <= 0) {
		set_exists(false);
	}
	// otherwise fitness is average of health stats
	else {
		fitness = static_cast<float>(nutrition + hydration + integrity) / 3e6f;
		// update age
		age++;
	}

	// return whether organism is stil alive
	return get_exists();
}

// determine distance and heading to closest food item
void GeneticSimulation::Organism::search_for_food(const ConsumableResourcePool& food)
{
	// return if not alive
	if (!get_exists()) return;

	// calculate and save heading to nearest food as well as hunger value
	sensory_data.set_food_heading(get_heading_to_nearest_resource(food));
	sensory_data.set_hunger(nutrition);
}

// determine distance and heading to closest water item
void GeneticSimulation::Organism::search_for_water(const ConsumableResourcePool& water)
{
	// return if not alive
	if (!get_exists()) return;

	// calculate and save heading to nearest water as well as thrist value
	sensory_data.set_water_heading(get_heading_to_nearest_resource(water));
	sensory_data.set_thirst(hydration);
}

// set heading (velocity) based on sensory data
void GeneticSimulation::Organism::think()
{
	// return if not alive
	if (!get_exists()) return;

	// make behavioural decision based on genotype and sensory data
	auto& decision = genotype.express_behaviour(sensory_data.get_data());
	// set velocity based on decision
	set_velocity(decision[0] * pi, phenotype.get_speed());
	// save memory item
	sensory_data.set_memory(decision[1]);
}

// update position
void GeneticSimulation::Organism::move()
{
	// return if not alive
	if (!get_exists()) return;

	// update position and wrap if out of bounds
	update_position_wrap();
}

// update graphical sprite
void GeneticSimulation::Organism::update_sprite(unsigned int fps)
{
	// return if not alive
	if (!get_exists()) return;

	// if gene transfer visual effect is active
	if (transfer_effect_time >= 0) {
		// increment effect time counter
		transfer_effect_time++;
		// if effect has lasted 1.5s worth of time steps, turn off
		if (transfer_effect_time > fps * 1.5f) {
			transfer_effect_time = -1;
		}
	}

	// set sprite color based on fitness
	set_sprite_color(calculate_color());
	// set sprite outline color based on whether gene transfer recently occurred
	set_sprite_outline_color(calculate_outline_color(fps * 1.5f));
}

// get index
unsigned int GeneticSimulation::Organism::get_index() const
{
	return index;
}

// get fitness
float GeneticSimulation::Organism::get_fitness() const
{
	return fitness;
}

// get age
unsigned int GeneticSimulation::Organism::get_age() const
{
	return age;
}

// manually set collision status
void GeneticSimulation::Organism::set_collision(unsigned int i)
{
	collisions[i] = 1;
}

// reset any properties not overwritten each time step
void GeneticSimulation::Organism::reset()
{
	// set nutrition, hydration, integrity and fitness to full
	nutrition = hydration = integrity = one_million;
	fitness = 1.f;
	// reset age
	age = 0;
	// reset collisions 
	for (auto& i : collisions) {
		i = 0;
	}
	// reset gene transfer and gene transfer effect
	genes_transferred = false;
	transfer_effect_time = -1;
}

// calculate color based on fitness
sf::Color GeneticSimulation::Organism::calculate_color()
{
	// set gradient from red to green based on fitness
	return calculate_gradient(sf::Color(193, 21, 21, 128), sf::Color(5, 252, 83, 128),
		static_cast<float>(min(one_million, max(0, min({ nutrition.load(), hydration.load(), integrity })))) / 1e6f);
}

// calculate outline color
sf::Color GeneticSimulation::Organism::calculate_outline_color(float transfer_effect_len)
{
	// normal outline color
	sf::Color normal_outline(138, 31, 89, 200);
	// set outline color as gradient based on gene transfer effect time if active
	if (transfer_effect_time >= 0) {
		auto effect_progress = static_cast<float>(transfer_effect_time) / transfer_effect_len;
		return calculate_gradient(sf::Color(5, 21, 252, 200), normal_outline, effect_progress);
	}
	else {
		return normal_outline;
	}
}

// get heading to nearest resource item
float GeneticSimulation::Organism::get_heading_to_nearest_resource(const ConsumableResourcePool& pool)
{
	// shortest distance
	auto shortest_distance = numeric_limits<float>::max();
	// heading to closest resource
	auto heading = 0.f;

	// position
	auto pos = get_position();

	// position of current resource
	sf::Vector2f resource_pos;
	// x, y and squared distances to current resource
	float d_x, d_y, d_2;
	// for each item in pool
	for (unsigned int i = 0; i < pool.get_max_size(); i++) {
		// if item exists
		if (pool[i].get_exists()) {
			// get position of item
			resource_pos = pool[i].get_position();
			// calculate x and y distances to resource
			d_x = pos.x - resource_pos.x;
			d_y = pos.y - resource_pos.y;
			// calculate squared distance to resource
			d_2 = d_x * d_x + d_y * d_y;
			// if new shortest distance is found
			if (d_2 < shortest_distance) {
				// record shortest distance
				shortest_distance = d_2;
				// record heading to current resource
				heading = atan2(d_y, d_x);
			}
		}
	}

	// return heading to closest resource
	return heading;
}