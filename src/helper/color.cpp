#include "color.h"
#include <algorithm>

using std::min;
using std::max;

// calulate a gradient between two colors
sf::Color GeneticSimulation::calculate_gradient(const sf::Color& color1, 
	const sf::Color& color2, float p)
{
	p = min(1.f, max(0.f, p));
	sf::Color result;
	result.r = color2.r * p + color1.r * (1 - p);
	result.g = color2.g * p + color1.g * (1 - p);
	result.b = color2.b * p + color1.b * (1 - p);
	result.a = color2.a * p + color1.a * (1 - p);
	return result;
}

// calulate a gradient between three colors
sf::Color GeneticSimulation::calculate_double_gradient(const sf::Color& color1, 
	const sf::Color& color2, const sf::Color& color3, float p)
{
	p *= 2;
	return (p < 1) ? calculate_gradient(color1, color2, p) : 
		calculate_gradient(color2, color3, p - 1);
}