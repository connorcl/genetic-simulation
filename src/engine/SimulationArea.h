#pragma once

#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

namespace GeneticSimulation
{
	// A 2-dimensional simulation space which is viewed via a graphical window
	class SimulationArea
	{
	public:

		// constructor
		SimulationArea(sf::Vector2u area_sz, sf::Vector2u window_sz, 
			const std::string& window_title, unsigned int frame_rate, 
			sf::RenderWindow& window, sf::Font& font);

		// set the location of the viewport
		void set_viewport_location(int x, int y);

		// pan the viewport by the given amount
		void pan_viewport(int x, int y);

		// set the zoom level of the viewport
		void set_viewport_zoom(float new_zoom_factor);

		// change the zoom factor of the viewport by the given amount
		void zoom_viewport(float factor);

		// set frame rate limit on or off
		void set_limit_frame_rate(bool limit);

		// toggle frame rate limit
		void toggle_limit_frame_rate();

		// draw a circle shape if it lies partially or wholly within the viewport
		void draw(sf::CircleShape& shape, sf::Vector2f position, float size);

		// draw information on the viewport (location, zoom, time, temperature)
		void draw_annotations(unsigned int time, float upper_temp, float lower_temp);

		// get area size
		sf::Vector2u get_size() const;

		// get viewport origin
		sf::Vector2i get_viewport_origin() const;

		// get viewport size
		sf::Vector2f get_viewport_size() const;

		// get whether frame rate is limited
		bool get_limit_frame_rate() const;

	private:

		// size of the area in cells
		sf::Vector2u area_size;
		// origin cell coordinates of the viewport
		sf::Vector2i viewport_origin;
		// size of the viewport in cells
		sf::Vector2f viewport_size;
		// zoom factor of viewport
		float zoom_factor;
		// whether frame rate limit is active
		bool limit_frame_rate;
		// normal frame rate
		unsigned int standard_frame_rate;
		// location and zoom info text
		sf::Text viewport_info;
		// time info text
		sf::Text current_time;
		// text showing temperature of upper latitude
		sf::Text upper_temperature;
		// coloured rectangle representing upper temperature
		sf::RectangleShape upper_temperature_color;
		// text showing temperature of lower latitude
		sf::Text lower_temperature;
		// coloured rectangle representing lower temperature
		sf::RectangleShape lower_temperature_color;
		// render window
		sf::RenderWindow& window;
		// font
		sf::Font& font;
	};
}