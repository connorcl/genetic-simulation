#include "SimulationObject.h"
#include <cmath>
#include <algorithm>

using std::min;
using std::max;

using namespace GeneticSimulation;

// constructor which takes the simulation area in which the object exists
GeneticSimulation::SimulationObject::SimulationObject(SimulationArea& area) :
	exists(false), size(0.f), wrap(false), position{ 0.f, 0.f },
	velocity{ 0.f, 0.f }, area(area) {}

// pure virtual destructor definition
GeneticSimulation::SimulationObject::~SimulationObject() {}

// update position based on velocity and stop at area edges
void GeneticSimulation::SimulationObject::update_position_bounded()
{
	// return if not alive
	if (!exists) return;

	// update position
	position += velocity;
	// set edge collision mode flag
	wrap = false;
	// calculate bounds
	auto area_size = area.get_size();
	sf::Vector2f bounds_max(area_size.x - size - 1.f, area_size.y - size - 1.f);
	sf::Vector2f bounds_min(size, size);
	// enforce bounds
	position.x = min(max(position.x, bounds_min.x), bounds_max.x);
	position.y = min(max(position.y, bounds_min.y), bounds_max.y);
}

// update position based on velocity and wrap if out of bounds
void GeneticSimulation::SimulationObject::update_position_wrap()
{
	// return if not alive
	if (!exists) return;

	// update position
	position += velocity;
	// set edge collision mode flag
	wrap = true;
	// calculate bounds
	auto area_size = area.get_size();
	sf::Vector2f bounds_max(area_size.x - 1.f, area_size.y - 1.f);
	sf::Vector2f bounds_min(0.f, 0.f);
	// wrap if out of bounds
	if (position.x > bounds_max.x) {
		position.x = bounds_min.x + (position.x - bounds_max.x);
	}
	else if (position.x < bounds_min.x) {
		position.x = bounds_max.x - (bounds_min.x - position.x);
	}
	if (position.y > bounds_max.y) {
		position.y = bounds_min.y + (position.y - bounds_max.y);
	}
	else if (position.y < bounds_min.y) {
		position.y = bounds_max.y - (bounds_min.y - position.y);
	}
}

// draw sprite on screen
void GeneticSimulation::SimulationObject::draw()
{
	// return if not alive
	if (!exists) return;
	
	// if organism may be in process or wrapping around
	if (wrap) {
		// calculate bounds
		auto area_size = area.get_size();
		sf::Vector2f bounds_max(area_size.x - size - 1.f, area_size.y - size - 1.f);
		sf::Vector2f bounds_min(size, size);
		// whether to draw in two positions
		bool currently_wrapping = false;
		// second position as sprite wraps around
		sf::Vector2f second_position = position;
		// calculate second position if necessary
		if (position.x > bounds_max.x) {
			currently_wrapping = true;
			second_position.x = -(area_size.x - 1.f - position.x);
		}
		else if (position.x < bounds_min.x) {
			currently_wrapping = true;
			second_position.x = area_size.x - 1.f + position.x;
		}
		if (position.y > bounds_max.y) {
			currently_wrapping = true;
			second_position.y = -(area_size.y - 1.f - position.y);
		}
		else if (position.y < bounds_min.y) {
			currently_wrapping = true;
			second_position.y = area_size.y - 1.f + position.y;
		}
		// draw sprite in second position if necessary
		if (currently_wrapping) {
			area.draw(sprite, second_position, size);
		}
	}

	// draw sprite
	area.draw(sprite, position, size);
}

// get whether object is allocated / alive
bool GeneticSimulation::SimulationObject::get_exists() const
{
	return exists;
}

// get sprite size
float GeneticSimulation::SimulationObject::get_size() const
{
	return size;
}

// get object position
sf::Vector2f GeneticSimulation::SimulationObject::get_position() const
{
	return position;
}

// get area size
sf::Vector2u GeneticSimulation::SimulationObject::get_area_size() const
{
	return area.get_size();
}

// set existence status
void GeneticSimulation::SimulationObject::set_exists(bool status)
{
	exists = status;
}

// set position
void GeneticSimulation::SimulationObject::set_position(sf::Vector2f new_pos)
{
	position = new_pos;
}

// set velocity
void GeneticSimulation::SimulationObject::set_velocity(sf::Vector2f new_vel)
{
	velocity = new_vel;
}

// set velocity based on heading and speed
void GeneticSimulation::SimulationObject::set_velocity(float heading, float speed)
{
	velocity.x = cos(heading) * speed;
	velocity.y = sin(heading) * speed;
}

// set sprite color
void GeneticSimulation::SimulationObject::set_sprite_color(sf::Color color)
{
	sprite.setFillColor(color);
}

// set sprite outline color
void GeneticSimulation::SimulationObject::set_sprite_outline_color(sf::Color color)
{
	sprite.setOutlineColor(color);
}

// set sprite outline thickness
void GeneticSimulation::SimulationObject::set_sprite_outline_thickness(float thickness)
{
	sprite.setOutlineThickness(thickness);
}

// set sprite size
void GeneticSimulation::SimulationObject::set_size(float new_size)
{
	size = new_size;
	sprite.setRadius(size);
	sprite.setOrigin(size, size);
}