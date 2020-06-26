#include "Simulation.h"
#include "helper/SignalLink.h"
#include "helper/benchmark_helper.h"
#include <random>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/filesystem.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

using std::default_random_engine;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::chrono::steady_clock;
using std::chrono::microseconds;
using std::chrono::duration_cast;
using std::min;
using std::max;
using std::string;
using std::to_string;

// constructor
GeneticSimulation::Simulation::Simulation(const Config& config) : initialized(false), config(config) {}

// initialize simulation by creating and initializing the necessary components
void GeneticSimulation::Simulation::init()
{
	// create random number generator
	default_random_engine rng(-config.random_seed_factor);

	// load font
	namespace fs = boost::filesystem;
	for (auto& p : vector<fs::path>{ "data", "../data", "./" }) {
		auto font_file_path = p / "font.ttf";
		if (font.loadFromFile(font_file_path.string())) break;
	}

	// set up simulation area
	area_ptr = make_unique<SimulationArea>(
		sf::Vector2u(config.area_width, config.area_height),
		sf::Vector2u(config.viewport_width, config.viewport_height),
		config.title,
		config.standard_framerate,
		window,
		font
	);
	area_ptr->set_limit_frame_rate(true);

	// set up planet
	planet_ptr = make_unique<Planet>();
	// precompute temperatures once if not benchmarking this
	if (config.run_mode != 2) planet_ptr->precompute_temperatures(config);

	// set up food pool
	food_pool_ptr = make_unique<ConsumableResourcePool>(
		config.food_pool_size,
		config.food_max_val,
		sf::Color(2, 33, 2, 192),
		config.food_pool_pos_margin,
		*area_ptr
	);
	food_pool_ptr->init_random(config.food_pool_init, rng);

	// set up water pool
	water_pool_ptr = make_unique<ConsumableResourcePool>(
		config.water_pool_size,
		config.water_max_val,
		sf::Color(8, 173, 214, 192),
		config.water_pool_pos_margin,
		*area_ptr
	);
	water_pool_ptr->init_random(config.water_pool_init, rng);

	// set up population
	population_ptr = make_unique<Population>(
		*area_ptr,
		*planet_ptr,
		*food_pool_ptr,
		*water_pool_ptr,
		config
	);
	population_ptr->init_random(config.population_init, rng);

	// record initialization
	initialized = true;
}

// run task based on run mode in config
void GeneticSimulation::Simulation::run()
{
	// return if not initialized
	if (!initialized) return;

	// run task based on run mode
	switch (config.run_mode) {
	case 0:
		// run mode 0: run simulation
		run_threaded();
		break;
	case 1:
		// run mode 1: benchmark simulation
		run_threaded(true);
		break;
	case 2:
		// run mode 2: benchmark temperature computation
		planet_ptr->precompute_temperatures(config, true);
		break;
	default:
		// run multithreaded by default
		run_threaded();
		break;
	}
}

// run simulation using at least 1 simulation thread and 1 render thread
void GeneticSimulation::Simulation::run_threaded(bool benchmark)
{
	// get number of simulation threads from config and set to number of hardware processors if 0
	auto num_simulation_threads = config.simulation_threads == 0 ? 
		boost::thread::hardware_concurrency() : 
		config.simulation_threads;

	// calculate number of organisms, food items and water items to process per thread
	unsigned int organisms_per_thread = config.population_size / num_simulation_threads + 1;
	unsigned int food_items_per_thread = config.food_pool_size / num_simulation_threads + 1;
	unsigned int water_items_per_thread = config.water_pool_size / num_simulation_threads + 1;

	// barriers for synchronizing simulation threads
	boost::barrier replication_begin_barrier(num_simulation_threads);
	boost::barrier replication_end_barrier(num_simulation_threads);
	boost::barrier end_of_timestep_barrier(num_simulation_threads);

	// signal links for synchronizing simulation threads with render thread
	SignalLink draw_resources_begin_signal_link(num_simulation_threads, 1);
	SignalLink draw_population_begin_signal_link(num_simulation_threads, 1);
	SignalLink draw_done_signal_link(1, num_simulation_threads, true);

	// remove SFML frame rate limit if benchmarking
	if (benchmark) {
		area_ptr->set_limit_frame_rate(false);
	}

	// create vector for pointers to simulation thread objects
	vector<unique_ptr<boost::thread>> simulation_threads;

	// start simulation threads
	for (unsigned int i = 0; i < num_simulation_threads; i++) {
		simulation_threads.push_back(make_unique<boost::thread>(
			// thread function
			[&, i] {
				// create thread-local random number generator
				default_random_engine rng(i * config.random_seed_factor);
				// calculate organism, food item and water item start and end indices for thread
				unsigned int organism_start = i * organisms_per_thread;
				unsigned int organism_end = (i + 1) * organisms_per_thread;
				unsigned int food_start = i * food_items_per_thread;
				unsigned int food_end = (i + 1) * food_items_per_thread;
				unsigned int water_start = i * water_items_per_thread;
				unsigned int water_end = (i + 1) * water_items_per_thread;
				// timestep counter
				unsigned int t = 0;
				// loop until thread is interrupted
				while (true) {
					/*
						Interact

						Parallelizable across population as genes are protected by mutexes

						Reads existence, fitness, age and position of every other organism so
						conflicts with replicate, update fitness and move which write these
					*/
					population_ptr->interact(organism_start, organism_end, rng);

					/*
						React to temperature

						Parallelizable across population as organisms only read and write own data,
						apart from precomputed temperature data which does not change
					*/
					population_ptr->react_to_temperature(organism_start, organism_end, t);

					// wait for render thread to signal it is finished its last iteration
					draw_done_signal_link.wait();

					/*
						Distribute resources

						Parallelizable across resource pools as organism nourish and hydrate are atomic,
						multiple items can safely update nutrition/hydration simultaneously

						Reads existence, position and size and may write nutrition/hydration of every
						organism, so conflicts with replicate, update fitness, move, update phenotype,
						search for resources and update sprite which write/read these in conflicting way
					*/
					population_ptr->nourish(food_start, food_end, rng);
					population_ptr->hydrate(water_start, water_end, rng);
					
					// notify render thread that drawing of resources may begin
					draw_resources_begin_signal_link.notify();

					// wait until all previous tasks are finished
					replication_begin_barrier.wait();

					/*
						Replicate

						Parallelizable across population as available slots queue is protected by a mutex

						Conflicts with all other tasks as it may reset any dead organism
					*/
					population_ptr->replicate(organism_start, organism_end, rng);

					// wait until all replication is done
					replication_end_barrier.wait();

					/*
						Update phenotypes

						Parallelizable across population as each organism only reads and writes own data
					*/
					population_ptr->update_phenotypes(organism_start, organism_end);

					/*
						Update fitness

						Parallelizable across population as available slots queue is protected by a mutex
					*/
					population_ptr->update_fitness(organism_start, organism_end);

					/*
						Search for resources

						Parallelizable across population as each organism only writes own sensory data

						Reads existence and position of every resource so conflicts with distribute resources
						which writes these
					*/
					population_ptr->search_for_food(organism_start, organism_end);
					population_ptr->search_for_water(organism_start, organism_end);

					/*
						Think

						Parallelizable across population as each organism only reads and writes own data
					*/
					population_ptr->think(organism_start, organism_end);

					/*
						Move

						Parallelizable across population as each organism only reads and writes own data
					*/
					population_ptr->move(organism_start, organism_end);

					/*
						Update sprites

						Parallelizable across population as each organism only reads and writes own data
					*/
					population_ptr->update_sprites(organism_start, organism_end);

					// signal that drawing of population may now begin
					draw_population_begin_signal_link.notify();

					// increment timestep counter
					t++;

					// synchronize at end of timestep
					end_of_timestep_barrier.wait();
				}
			}
			// end of thread function
		));
	}

	// start main render loop in main thread
	main_render_loop(draw_resources_begin_signal_link, draw_population_begin_signal_link,
		draw_done_signal_link, num_simulation_threads, benchmark);

	// once main render loop has finished (window was closed) interrupt and join all simulation threads
	for (auto& t_ptr : simulation_threads) {
		t_ptr->interrupt();
		t_ptr->join();
	}
}

// main render loop for simulation
void GeneticSimulation::Simulation::main_render_loop(SignalLink& draw_resources_begin_signal_link, 
	SignalLink& draw_population_begin_signal_link, SignalLink& draw_done_signal_link,
	unsigned int num_simulation_threads, bool benchmark)
{
	// timestep counter
	unsigned int t = 0;
	// time points for timing frames
	steady_clock::time_point start, end, handle_events_start, handle_events_end;
	// duration of current frame, and sum time and count of non-framerate limited frames
	unsigned long long frame_time = 0;
	unsigned long long non_limited_frame_time_sum = 0;
	unsigned int non_limited_frame_count = 0;
	// whether framerate is currently limited and whether to draw current frame
	bool limit_framerate = !benchmark;
	bool draw = true;

	// record of frame times for benchmarking
	vector<unsigned long long> frame_times;
	// allocate space for frame times if benchmarking
	if (benchmark) {
		frame_times.resize(config.simulation_benchmark_timesteps);
	}

	// main loop for drawing and event handling
	while (window.isOpen()) {
		// start timing frame
		start = steady_clock::now();

		// if at least one frame has completed
		if (t > 0) {
			// record frame time if benchmarking
			if (benchmark && t <= config.simulation_benchmark_timesteps) {
				frame_times[t - 1] = frame_time;
			}
			// close window and exit loop if benchmark is complete
			if (benchmark && t >= config.simulation_benchmark_timesteps) {
				window.close();
				continue;
			}
			// add frame time to non-framerate-limited frame sum time if appropriate
			non_limited_frame_time_sum += frame_time * (limit_framerate ? 0 : 1);
		}

		// handle events
		handle_events_start = steady_clock::now();
		handle_events(!benchmark);
		handle_events_end = steady_clock::now();

		// get whether framerate is limited for this frame
		limit_framerate = benchmark ? false : area_ptr->get_limit_frame_rate();
		// determine whether to draw this frame
		draw = calculate_draw(t, limit_framerate, non_limited_frame_time_sum, 
			non_limited_frame_count, config.performance_framerate);

		// clear window
		if (draw) {
			window.clear(sf::Color(config.background_color));
		}
		// wait until food and water consumption is done
		draw_resources_begin_signal_link.wait();
		// draw food and water
		if (draw) {
			water_pool_ptr->draw();
			food_pool_ptr->draw();
		}
		// wait until population can be drawn
		draw_population_begin_signal_link.wait();
		// draw population, overlay info annotations and display
		if (draw) {
			population_ptr->draw();
			auto viewport_origin = area_ptr->get_viewport_origin();
			auto upper_temperature = planet_ptr->get_temperature(viewport_origin.y, t % config.orbital_period);
			auto lower_temperature = planet_ptr->get_temperature(max(0u, min(area_ptr->get_size().y - 1u,
				viewport_origin.y + static_cast<int>(area_ptr->get_viewport_size().y) - 1u)), t % config.orbital_period);
			area_ptr->draw_annotations(t, upper_temperature, lower_temperature);
			window.display();
		}
		// signal that iteration is done
		draw_done_signal_link.notify();

		// increment timestep and frame counters
		t++;
		non_limited_frame_count += (limit_framerate ? 0 : 1);

		// calculate frame time
		end = steady_clock::now();
		frame_time = duration_cast<microseconds>((end - start) - 
			(handle_events_end - handle_events_start)).count();
	}

	// write benchmark results
	if (benchmark && t >= config.simulation_benchmark_timesteps) {
		write_benchmark_results(frame_times, 
			"frame_microseconds_" + to_string(num_simulation_threads) + "_simulation_threads",
			"benchmark_results_" + to_string(num_simulation_threads) + "_simulation_threads.csv", config.results_path);
	}
}

// calculate whether to draw current frame
inline bool GeneticSimulation::Simulation::calculate_draw(unsigned int timestep, bool limit_framerate,
	unsigned long long frame_sum_us, unsigned int frame_count, unsigned int target_framerate)
{
	// always draw if framerate is automatically limited or no data is available
	if (limit_framerate || frame_sum_us == 0 || frame_count == 0) return true;
	// calculate framerate
	double framerate = frame_count / (static_cast<long double>(frame_sum_us) / 1e6);
	// calculate how often to draw to achieve target framerate
	unsigned int draw_every = max(1u, static_cast<unsigned int>(round(framerate / max(1u, target_framerate))));
	// return whether to draw this frame
	return timestep % draw_every == 0;
}

// handle keypresses and window closure
void GeneticSimulation::Simulation::handle_events(bool allow_framerate_toggle)
{
	// pan / zoom viewport if relevant key is pressed
	if (area_ptr->get_limit_frame_rate()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			area_ptr->pan_viewport(-4, 0);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			area_ptr->pan_viewport(4, 0);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			area_ptr->pan_viewport(0, -4);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			area_ptr->pan_viewport(0, 4);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			area_ptr->zoom_viewport(0.01f);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			area_ptr->zoom_viewport(-0.01f);
		}
	}

	// poll for events
	while (window.pollEvent(event)) {
		switch (event.type) {
		case sf::Event::Closed:
			window.close();
			break;
		case sf::Event::KeyPressed:
			if (event.key.code == sf::Keyboard::F && allow_framerate_toggle) {
				area_ptr->toggle_limit_frame_rate();
			}
			break;
		default:
			break;
		}
	}
}