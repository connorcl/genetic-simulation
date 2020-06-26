#pragma once

#include "helper/platform.h"
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace GeneticSimulation
{
	// stores simulation configuration options
	class Config
	{
	public:

		// initialize config from command line and config file
		void init(int argc, char* argv[]);

		// compute options
		unsigned int run_mode;
		unsigned int performance_framerate;
		unsigned int standard_framerate;
		unsigned int simulation_threads;
#ifdef GPU_SUPPORT
		bool precompute_temperatures_gpu;
#endif
		unsigned int precompute_temperatures_cpu_threads;
		unsigned int simulation_benchmark_timesteps;
		unsigned int planet_benchmark_samples;
		int random_seed_factor;
		std::string results_path;

		// area options
		unsigned int area_width;
		unsigned int area_height;
		float latitude_range;
		unsigned int viewport_width;
		unsigned int viewport_height;
		std::string title;
		uint32_t background_color;

		// planet options
		unsigned int orbital_period;
		double orbit_center_offset_x;
		double orbit_center_offset_y;
		double orbit_radius_x;
		double orbit_radius_y;
		double orbit_rotation;
		double star_luminosity;
		double albedo;
		double axial_tilt;
		double radius;
		double atmosphere_optical_thickness;
		double temperature_moderation_factor;
		double temperature_moderation_bias;

		// food pool options
		unsigned int food_pool_size;
		unsigned int food_max_val;
		float food_pool_pos_margin;
		unsigned int food_pool_init;

		// water pool options
		unsigned int water_pool_size;
		unsigned int water_max_val;
		float water_pool_pos_margin;
		unsigned int water_pool_init;

		// population options
		unsigned int population_size;
		float population_pos_margin;
		float area_of_influence_mean;
		float area_of_influence_sigma;
		float speed_mean;
		float speed_sigma;
		float health_rate_mean;
		float health_rate_sigma;
		float ideal_temp_mean;
		float ideal_temp_sigma;
		float temp_range_mean;
		float temp_range_sigma;
		float behaviour_net_weight_range;
		float behaviour_net_weight_range_bias;
		unsigned int behaviour_net_layer_1_units;
		unsigned int behaviour_net_layer_2_units;
		unsigned int population_init;
		float replication_rate;
		float behaviour_net_mutation_prob;
		float behaviour_net_mutation_sigma;
		float trait_genes_mutation_prob;
		float trait_genes_mutation_sigma;

	private:

		// set up command line options description
		void set_up_options_description(boost::program_options::options_description& desc);

		// parse program command line and store in variables map
		bool parse_command_line(int argc, char* argv[], const boost::program_options::options_description& desc,
			boost::program_options::variables_map& vm);

		// get config file location from command line options or default locations (empty if none found)
		boost::filesystem::path get_config_file_location(bool valid_options, const boost::program_options::variables_map& vm);

		// load config from file
		void parse_config_file(const std::string& config_file);

		// parse command line options (excluding config file)
		void parse_command_line_options(const boost::program_options::variables_map& vm);

		// convert a 3-byte hex string into a 32-bit color value
		uint32_t parse_hex_color(const std::string& color_string);

		// get a numerical config option from the property tree
		template<class T>
		T get_numerical_option(const boost::property_tree::ptree& pt, const std::string& path, T min_val, T max_val, T default_val)
		{
			T value;
			try {
				value = std::min(max_val, std::max(min_val, pt.get<T>(path)));
			}
			catch (boost::property_tree::ptree_error& e) {
				value = default_val;
				std::cerr << "Config value not found: " << e.what() << "\n";
			}
			return value;
		}

		// get a config option of any type from the property tree
		template<class T>
		T get_option(const boost::property_tree::ptree& pt, const std::string& path, T default_val)
		{
			T value;
			try {
				value = pt.get<T>(path);
			}
			catch (boost::property_tree::ptree_error& e) {
				value = default_val;
				std::cerr << "Config value not found: " << e.what() << "\n";
			}
			return value;
		}
	};
}