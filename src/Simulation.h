#pragma once

#include "Planet.h"
#include "engine/SimulationArea.h"
#include "ConsumableResourcePool.h"
#include "Population.h"
#include "Config.h"
#include "helper/SignalLink.h"
#include <memory>
#include <SFML/Graphics.hpp>

namespace GeneticSimulation
{
	// encapsulates the entire simulation
	class Simulation
	{
	public:

		// constructor
		explicit Simulation(const Config& config);

		// initialize simulation by creating and initializing the necessary components
		void init();

		// run task based on run mode in config
		void run();

	private:

		// run simulation using at least 1 simulation thread and 1 render thread
		void run_threaded(bool benchmark = false);

		// main render loop for simulation
		void main_render_loop(SignalLink& draw_resources_begin_signal_link,
			SignalLink& draw_population_begin_signal_link, 
			SignalLink& draw_done_signal_link, unsigned int simulation_threads, bool benchmark = false);

		// calculate whether to draw current frame
		inline bool calculate_draw(unsigned int timestep, bool limit_framerate,
			unsigned long long frame_sum, unsigned int frame_count, unsigned int target_framerate);

		// handle keypresses and window closure
		void handle_events(bool allow_framerate_toggle = true);
	
		// whether components have been initialized
		bool initialized;
		// graphical window
		sf::RenderWindow window;
		// event
		sf::Event event;
		// font
		sf::Font font;
		// Pointer to planet
		std::unique_ptr<Planet> planet_ptr;
		// Pointer to simulation area
		std::unique_ptr<SimulationArea> area_ptr;
		// Pointer to food resource pool
		std::unique_ptr<ConsumableResourcePool> food_pool_ptr;
		// Pointer to water resource pool
		std::unique_ptr<ConsumableResourcePool> water_pool_ptr;
		// Pointer to population
		std::unique_ptr<Population> population_ptr;
		// Reference to config
		const Config& config;
	};
}