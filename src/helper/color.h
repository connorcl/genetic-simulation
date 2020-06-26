#pragma once

#include <SFML/Graphics/Color.hpp>

namespace GeneticSimulation
{
	// calculate a gradient between two colors
	sf::Color calculate_gradient(const sf::Color& color1, const sf::Color& color2, float p);

	// calculate a gradient between three colors
	sf::Color calculate_double_gradient(const sf::Color& color1, const sf::Color& color2,
		const sf::Color& color3, float p);
}