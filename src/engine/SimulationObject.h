#pragma once

#include "SimulationArea.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

namespace GeneticSimulation
{
	// Abstract base class for an object that is part of a pool 
	// of similar objects and exists within a 2D simulation area
	class SimulationObject
	{
	public:

		// constructor which takes the simulation area in which the object exists
		explicit SimulationObject(SimulationArea& area);

		// pure virtual destructor making this an abstract base class
		virtual ~SimulationObject() = 0;

		// update position based on velocity and stop at area edges
		void update_position_bounded();

		// update position based on velocity and wrap if out of bounds
		void update_position_wrap();

		// draw sprite on screen
		void draw();

		// get whether object is allocated / alive
		bool get_exists() const;

		// get sprite size
		float get_size() const;

		// get object position
		sf::Vector2f get_position() const;

	protected:

		// get area size
		sf::Vector2u get_area_size() const;

		// set existence status
		void set_exists(bool status); 

		// set position
		void set_position(sf::Vector2f new_pos);

		// set velocity
		void set_velocity(sf::Vector2f new_vel);

		// set velocity based on heading and speed
		void set_velocity(float heading, float speed);

		// set sprite color
		void set_sprite_color(sf::Color color);

		// set sprite outline color
		void set_sprite_outline_color(sf::Color color);

		// set sprite outline thickness
		void set_sprite_outline_thickness(float thickness);

		// set sprite size
		void set_size(float new_size);

	private:

		// whether object is active / allocated in its pool
		bool exists;
		// size of sprite
		float size;
		// whether last movement was potentially wrapping
		bool wrap;
		// circular sprite
		sf::CircleShape sprite;
		// position of the object
		sf::Vector2f position;
		// velocity of the object
		sf::Vector2f velocity;
		// reference to area in which object exists
		SimulationArea& area;
	};
}