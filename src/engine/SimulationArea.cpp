#include "SimulationArea.h"
#include "../helper/color.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include <sstream>
#include <iomanip>

using std::min;
using std::max;
using std::numeric_limits;
using std::string;
using std::to_string;
using std::stringstream;
using std::fixed;
using std::setprecision;

using namespace GeneticSimulation;

// constructor
GeneticSimulation::SimulationArea::SimulationArea(sf::Vector2u area_sz, sf::Vector2u window_sz, 
	const string& window_title, unsigned int frame_rate, sf::RenderWindow& window, sf::Font& font) :
	viewport_origin(0, 0), zoom_factor(1.f), limit_frame_rate(true),
	standard_frame_rate(frame_rate), window(window), font(font)
{
	// ensure area and viewport sizes are at least 300 by 300 and limit viewport size to area size
	area_size.x = max(300u, area_sz.x);
	area_size.y = max(300u, area_sz.y);
	viewport_size.x = min(area_size.x, max(300u, window_sz.x));
	viewport_size.y = min(area_size.y, max(300u, window_sz.y));
	// (re)create window
	window.create(sf::VideoMode(viewport_size.x, viewport_size.y), window_title);
	// set up viewport info text element
	viewport_info.setFont(font);
	viewport_info.setCharacterSize(14);
	viewport_info.setFillColor(sf::Color::Black);
	viewport_info.setPosition(10, 10);
	// set up time text element
	current_time.setFont(font);
	current_time.setCharacterSize(20);
	current_time.setFillColor(sf::Color::Black);
	current_time.setPosition(10, viewport_size.y - 30);
	// set up upper temperature text element
	upper_temperature.setFont(font);
	upper_temperature.setCharacterSize(20);
	upper_temperature.setFillColor(sf::Color::Black);
	upper_temperature.setPosition(viewport_size.x - 120, 7);
	// set up upper temperature color element
	upper_temperature_color.setSize(sf::Vector2f(30, 20));
	upper_temperature_color.setPosition(viewport_size.x - 40, 10);
	upper_temperature_color.setOutlineThickness(2);
	upper_temperature_color.setOutlineColor(sf::Color::Black);
	// set up lower temperature text element
	lower_temperature.setFont(font);
	lower_temperature.setCharacterSize(20);
	lower_temperature.setFillColor(sf::Color::Black);
	lower_temperature.setPosition(viewport_size.x - 120, viewport_size.y - 33);
	// set up lower temperature color element
	lower_temperature_color.setSize(sf::Vector2f(30, 20));
	lower_temperature_color.setPosition(viewport_size.x - 40, viewport_size.y -30);
	lower_temperature_color.setOutlineThickness(2);
	lower_temperature_color.setOutlineColor(sf::Color::Black);
}

// set the location of the viewport
void GeneticSimulation::SimulationArea::set_viewport_location(int x, int y)
{
	// maximum viewport origin is area size minus viewport size
	int max_x = area_size.x - ceil(viewport_size.x);
	int max_y = area_size.y - ceil(viewport_size.y);
	// move viewport and clamp origin between 0 and max
	viewport_origin.x = min(max_x, max(0, x));
	viewport_origin.y = min(max_y, max(0, y));
}

// pan the viewport by the given amount
void GeneticSimulation::SimulationArea::pan_viewport(int x, int y)
{
	set_viewport_location(viewport_origin.x + x, viewport_origin.y + y);
}

// set the zoom level of the viewport
void GeneticSimulation::SimulationArea::set_viewport_zoom(float new_zoom_factor)
{
	// get current resolution of window
	auto window_res = window.getSize();
	// calculate minimum zoom factor
	float zoom_min = max(static_cast<float>(window_res.x) / area_size.x,
		static_cast<float>(window_res.y) / area_size.y);

	// set zoom factor, clamping between min and 3
	zoom_factor = min(3.f, max(zoom_min, new_zoom_factor));

	// calculate new viewport size (viewport size is window res / zoom factor)
	sf::Vector2f new_viewport_size(static_cast<float>(window_res.x) / zoom_factor, 
								   static_cast<float>(window_res.y) / zoom_factor);
	// calculate change in viewport size
	sf::Vector2f viewport_size_delta = new_viewport_size - viewport_size;
	// update viewport size
	viewport_size = new_viewport_size;

	// move viewport so as to zoom around center of viewport
	pan_viewport(-round(viewport_size_delta.x) / 2, -round(viewport_size_delta.y) / 2);
}

// change the zoom factor of the viewport by the given amount
void GeneticSimulation::SimulationArea::zoom_viewport(float factor)
{
	set_viewport_zoom(zoom_factor + factor);
}

// set frame rate limit on or off
void GeneticSimulation::SimulationArea::set_limit_frame_rate(bool limit)
{
	limit_frame_rate = limit;
	window.setFramerateLimit(limit ? standard_frame_rate : numeric_limits<unsigned int>::max());
}

// toggle frame rate limit
void GeneticSimulation::SimulationArea::toggle_limit_frame_rate()
{
	set_limit_frame_rate(!limit_frame_rate);
}

// draw a circle shape if it lies partially or wholly within the viewport
void GeneticSimulation::SimulationArea::draw(sf::CircleShape& shape, sf::Vector2f position, float size)
{
	// get window resoultion
	auto window_res = window.getSize();

	// calculate pixel position of shape relative to viewport
	sf::Vector2f relative_position((position.x - viewport_origin.x) * zoom_factor, 
								   (position.y - viewport_origin.y) * zoom_factor);

	// if shape is partially or wholly within viewport
	if (relative_position.x + size >= 0 && relative_position.x - size < window_res.x
		&& relative_position.y + size >= 0 && relative_position.y - size < window_res.y)
	{
		// set position to relative position
		shape.setPosition(relative_position);
		// set scale to zoom factor
		shape.setScale(zoom_factor, zoom_factor);
		// draw shape on window
		window.draw(shape);
	}
}

// draw information on the viewport (location, zoom, time, temperature)
void GeneticSimulation::SimulationArea::draw_annotations(unsigned int time, 
	float upper_temp, float lower_temp)
{
	// generate and set string for viewport info
	stringstream viewport_info_stream;
	viewport_info_stream << "Location: " << viewport_origin.x << ", " << viewport_origin.y << "\n";
	viewport_info_stream << "Zoom: " << fixed << setprecision(2) << zoom_factor << "x";
	viewport_info.setString(viewport_info_stream.str());
	
	// set time string
	current_time.setString(to_string(time / standard_frame_rate));
	
	// set upper temperature string and color
	stringstream upper_temperature_stream;
	upper_temperature_stream << fixed << setprecision(1) << upper_temp - 273.15 << "C";
	upper_temperature.setString(upper_temperature_stream.str());
	upper_temperature_color.setFillColor(calculate_double_gradient(sf::Color::Blue, 
		sf::Color::Yellow, sf::Color::Red, (upper_temp - 200.f) / 200.f));
	
	// set lower temperature string and color
	stringstream lower_temperature_stream;
	lower_temperature_stream << fixed << setprecision(1) << lower_temp - 273.15 << "C";
	lower_temperature.setString(lower_temperature_stream.str());
	lower_temperature_color.setFillColor(calculate_double_gradient(sf::Color::Blue,
		sf::Color::Yellow, sf::Color::Red, (lower_temp - 200.f) / 200.f));
	
	// draw
	window.draw(viewport_info);
	window.draw(current_time);
	window.draw(upper_temperature);
	window.draw(upper_temperature_color);
	window.draw(lower_temperature);
	window.draw(lower_temperature_color);
}

// get area size
sf::Vector2u GeneticSimulation::SimulationArea::get_size() const
{
	return area_size;
}

// get viewport origin
sf::Vector2i GeneticSimulation::SimulationArea::get_viewport_origin() const
{
	return viewport_origin;
}

// get viewport size
sf::Vector2f GeneticSimulation::SimulationArea::get_viewport_size() const
{
	return viewport_size;
}

// get whether frame rate is limited
bool GeneticSimulation::SimulationArea::get_limit_frame_rate() const
{
	return limit_frame_rate;
}