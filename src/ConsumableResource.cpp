#include "ConsumableResource.h"

using namespace GeneticSimulation;

// constructor which takes the color of the item and the area in which it exists
GeneticSimulation::ConsumableResource::ConsumableResource(sf::Color color, SimulationArea& area) :
	SimulationObject(area), value(0)
{
	set_sprite_color(color);
	set_sprite_outline_thickness(-1.f);
	set_sprite_outline_color(sf::Color(138, 31, 89, 200));
}

// initialize the item with a value and a position
void GeneticSimulation::ConsumableResource::init(unsigned int val, unsigned int max_val, sf::Vector2f pos)
{
	// set value
	value = val;
	// set position
	set_position(pos); 
	// set size
	set_size((static_cast<float>(value) / static_cast<float>(max_val)) * 6.f);
	// set existance to true
	set_exists(true);
}

// consume the resource's energy
unsigned int GeneticSimulation::ConsumableResource::consume() 
{
	set_exists(false);
	return value;
}