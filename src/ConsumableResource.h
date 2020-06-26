#pragma once

#include "engine/SimulationObject.h"
#include "engine/SimulationArea.h"
#include <SFML/Graphics.hpp>

namespace GeneticSimulation
{
	// A simulation object representing a consumable
	// resource item such as food or water
	class ConsumableResource : public SimulationObject
	{
	public:

		// constructor which takes the color of the 
		// item and the area in which it exists
		ConsumableResource(sf::Color color, SimulationArea& area);

		// initialize the item with a value and a position
		void init(unsigned int val, unsigned int max_val, sf::Vector2f pos);

		// consume the resource's energy
		unsigned int consume();

	private:

		// value of resource
		unsigned int value;
	};
}