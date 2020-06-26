#include "Config.h"
#include "helper/platform.h"
#include <cstdlib>
#include <string>
#include <limits>
#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using std::string;
using std::numeric_limits;
using std::cout;
using std::cerr;
using std::vector;

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

// initialize config from command line and config file
void GeneticSimulation::Config::init(int argc, char* argv[])
{
	// set up program options and descriptions
	po::options_description desc("Recognised options");
	set_up_options_description(desc);

	// parse command line into variables map
	po::variables_map vm;
	auto valid_options_given = parse_command_line(argc, argv, desc, vm);

	// attempt to set config file path from command line or known locations
	auto config_file_path = get_config_file_location(valid_options_given, vm);
	
	// load configuration options from config file (if path is invalid/empty, default values will be used)
	parse_config_file(config_file_path.string());

	// parse rest of command line arguments (and override config file)
	if (valid_options_given) {
		parse_command_line_options(vm);
	}
}

// set up command line options description
void GeneticSimulation::Config::set_up_options_description(po::options_description& desc)
{
	// add descriptions for each option
	desc.add_options()
		("help,h", "Produce help message")
		("run_mode,m", po::value<unsigned int>(),
			"Select which task to run: \n"
			"0 = run simulation\n"
			"1 = benchmark simulation\n"
			"2 = benchmark temperature computation")
		("config_file,i", po::value<string>(), "Set path to config file")
		("simulation_threads,s", po::value<unsigned int>(), "Set number of simulation threads")
#ifdef GPU_SUPPORT
		("planet_gpu,g", po::value<bool>(), "Set whether to precompute temperatures using GPU")
#endif
		("planet_cpu_threads,c", po::value<unsigned int>(),
			"Set number of threads to use when precomputing temperatures on CPU")
		("benchmark_timesteps,t", po::value<unsigned int>(), "Set number of timesteps in simulation benchmark period")
		("planet_benchmark_samples,p", po::value<unsigned int>(),
			"Set number of samples when benchmarking temperature computation");
}

// parse program command line and store in variables map
bool GeneticSimulation::Config::parse_command_line(int argc, char* argv[], 
	const po::options_description& desc, po::variables_map& vm)
{
	// whether a valid set of options were passed on the command line
	bool valid_options_given = true;
	// attempt to parse command line
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch (const po::error& e) {
		// if parsing fails, log error and do not use command line arguments
		cerr << "Parsing command line options failed: " << e.what() << "\n";
		valid_options_given = false;
	}
	// if help option given, print usage info and exit
	if (valid_options_given && vm.count("help")) {
		cout << desc << "\n";
		exit(0);
	}
	// return whether parsing was successful
	return valid_options_given;
}

// get config file location from command line options or default locations (empty if none found)
fs::path GeneticSimulation::Config::get_config_file_location(bool valid_options, const po::variables_map& vm)
{
	// config file path
	fs::path config_file_path;
	// check command line options for a specified config file location
	if (valid_options && vm.count("config_file")) {
		config_file_path = vm["config_file"].as<string>();
	}
	else {
		// default locations
		vector<fs::path> default_config_locations = { "./", "config/", "../config/" };
		// path being checked
		fs::path current_config_path;
		// check for config file in default locations
		for (auto& p : default_config_locations) {
			current_config_path = p / "config.ini";
			if (fs::exists(current_config_path)) {
				config_file_path = current_config_path;
				break;
			}
		}
	}
	// output error message if no config file was specified or found
	if (config_file_path.empty()) {
		cerr << "No config file was specified or found, falling back to internal default config\n";
	}
	// return path
	return config_file_path;
}

// load config from file
void GeneticSimulation::Config::parse_config_file(const std::string& config_file)
{
	// property tree of config options
	pt::ptree config_pt;

	// check if config file was specified
	if (!config_file.empty()) {
		// parse config file
		try {
			cout << "Reading config file " << config_file << "\n";
			pt::ini_parser::read_ini(config_file, config_pt);
		}
		catch (const pt::ini_parser::ini_parser_error& e) {
			// if reading config file fails, log error
			cerr << "Reading config file failed: " << e.message() << "\n";
		}
	}

	// set compute options
	run_mode = get_option<unsigned int>(config_pt, "Compute.run_mode", 0);
	performance_framerate = get_numerical_option<unsigned int>(config_pt, "Compute.performance_framerate", 1, 250, 36);
	standard_framerate = get_numerical_option<unsigned int>(config_pt, "Compute.standard_framerate", 1, 250, 90);
	simulation_threads = get_numerical_option<unsigned int>(config_pt, "Compute.simulation_threads", 0, 256, 4);
#ifdef GPU_SUPPORT
	precompute_temperatures_gpu = get_option<bool>(config_pt, "Compute.precompute_temperatures_gpu", false);
#endif
	precompute_temperatures_cpu_threads = get_numerical_option<unsigned int>(config_pt,
		"Compute.precompute_temperatures_cpu_threads", 0, 256, 4);
	simulation_benchmark_timesteps = get_numerical_option<unsigned int>(config_pt, 
		"Compute.simulation_benchmark_timesteps", 1u, 1e6, 30000);
	planet_benchmark_samples = get_numerical_option<unsigned int>(config_pt, 
		"Compute.planet_benchmark_samples", 1, 1e3, 50);
	random_seed_factor = get_numerical_option<int>(config_pt, "Compute.random_seed_factor", -1000000, 1000000, 1);
	results_path = get_option<std::string>(config_pt, "Compute.results_path", "./");

	// set area options
	area_width = get_numerical_option<unsigned int>(config_pt, "Area.width", 300, 1e4, 1600);
	area_height = get_numerical_option<unsigned int>(config_pt, "Area.height", 300, 1e4, 1200);
	latitude_range = get_numerical_option<float>(config_pt, "Area.latitude_range", 1, 90, 90);
	viewport_width = get_numerical_option<unsigned int>(config_pt, "Area.viewport_width", 300, 1e4, 800);
	viewport_height = get_numerical_option<unsigned int>(config_pt, "Area.viewport_height", 300, 1e4, 600);
	title = get_option<string>(config_pt, "Area.title", "Genetic Simulation");
	background_color = parse_hex_color(get_option<string>(config_pt, "Area.background_color", "ffffff"));

	// set planet options
	orbital_period = get_numerical_option<unsigned int>(config_pt, "Planet.orbital_period", 1e3, 1e6, 36000);
	orbit_center_offset_x = get_numerical_option<double>(config_pt, "Planet.orbit_center_offset_x",
		0, numeric_limits<double>::max(), 0);
	orbit_center_offset_y = get_numerical_option<double>(config_pt, "Planet.orbit_center_offset_y",
		0, numeric_limits<double>::max(), 0);
	orbit_radius_x = get_numerical_option<double>(config_pt, "Planet.orbit_radius_x",
		1e8, numeric_limits<double>::max(), 172e9);
	orbit_radius_y = get_numerical_option<double>(config_pt, "Planet.orbit_radius_y",
		1e8, numeric_limits<double>::max(), 138e9);
	orbit_rotation = get_numerical_option<double>(config_pt, "Planet.orbit_rotation",
		numeric_limits<double>::min(), numeric_limits<double>::max(), 0);
	star_luminosity = get_numerical_option<double>(config_pt, "Planet.star_luminosity",
		0, numeric_limits<double>::max(), 3.846e26);
	albedo = get_numerical_option<double>(config_pt, "Planet.albedo", 0, 1, 0.29);
	axial_tilt = get_numerical_option<double>(config_pt, "Planet.axial_tilt", 0, 45, 23);
	radius = get_numerical_option<double>(config_pt, "Planet.radius", 1e3, 1e7, 6371e3);
	atmosphere_optical_thickness = get_numerical_option<double>(config_pt, "Planet.atmosphere_optical_thickness", 0, 10, 1.3);
	temperature_moderation_factor = get_numerical_option<double>(config_pt, "Planet.temperature_moderation_factor", 1, 10, 4.0);
	temperature_moderation_bias = get_numerical_option<double>(config_pt, "Planet.temperature_moderation_bias", 0, 1, 0.8);

	// set food options
	food_pool_size = get_numerical_option<unsigned int>(config_pt, "Food.pool_size", 1, 8192, 148);
	food_max_val = get_numerical_option<unsigned int>(config_pt, "Food.max_val", 10000, 1000000, 250000);
	food_pool_pos_margin = get_numerical_option<float>(config_pt, "Food.pool_pos_margin", 0, 150, 10);
	food_pool_init = get_numerical_option<unsigned int>(config_pt, "Food.pool_init", 1, 8192, 148);

	// set water options
	water_pool_size = get_numerical_option<unsigned int>(config_pt, "Water.pool_size", 1, 8192, 148);
	water_max_val = get_numerical_option<unsigned int>(config_pt, "Water.max_val", 10000, 1000000, 250000);
	water_pool_pos_margin = get_numerical_option<float>(config_pt, "Water.pool_pos_margin", 0, 150, 10);
	water_pool_init = get_numerical_option<unsigned int>(config_pt, "Water.pool_init", 1, 8192, 148);

	// set population options
	population_size = get_numerical_option<unsigned int>(config_pt, "Population.pool_size", 1, 8192, 512);
	population_pos_margin = get_numerical_option<float>(config_pt, "Population.pool_pos_margin", 0, 150, 20);
	area_of_influence_mean = get_numerical_option<float>(config_pt, "Population.area_of_influence_mean", 1, 100, 8);
	area_of_influence_sigma = get_numerical_option<float>(config_pt, "Population.area_of_influence_sigma", 0, area_of_influence_mean / 4, 2);
	speed_mean = get_numerical_option<float>(config_pt, "Population.speed_mean", 0.1f, 100, 1);
	speed_sigma = get_numerical_option<float>(config_pt, "Population.speed_sigma", 0, speed_mean / 5, 0.1f);
	health_rate_mean = get_numerical_option<float>(config_pt, "Population.health_rate_mean", 1, 1e6f, 220);
	health_rate_sigma = get_numerical_option<float>(config_pt, "Population.health_rate_sigma", 0, health_rate_mean / 5, 30);
	ideal_temp_mean = get_numerical_option<float>(config_pt, "Population.ideal_temp_mean", 0, 1e3, 260);
	ideal_temp_sigma = get_numerical_option<float>(config_pt, "Population.ideal_temp_sigma", 0, ideal_temp_mean / 5, 30);
	temp_range_mean = get_numerical_option<float>(config_pt, "Population.temp_range_mean", 0, 100, 10);
	temp_range_sigma = get_numerical_option<float>(config_pt, "Population.temp_range_sigma", 0, temp_range_mean / 5, 2);
	behaviour_net_weight_range = get_numerical_option<float>(config_pt, "Population.behaviour_net_weight_range", 1e-4f, 10, 2);
	behaviour_net_weight_range_bias = get_numerical_option<float>(config_pt, "Population.behaviour_net_weight_range_bias", 1, 10, 1);
	behaviour_net_layer_1_units = get_numerical_option<unsigned int>(config_pt, "Population.behaviour_net_layer_1_units", 1, 128, 16);
	behaviour_net_layer_2_units = get_numerical_option<unsigned int>(config_pt, "Population.behaviour_net_layer_2_units", 1, 128, 8);
	population_init = get_numerical_option<unsigned int>(config_pt, "Population.pool_init", 1, 8192, 512);
	replication_rate = get_numerical_option<float>(config_pt, "Population.replication_rate", 0, 1, 0.0001f);
	behaviour_net_mutation_prob = get_numerical_option<float>(config_pt, "Population.behaviour_net_mutation_prob", 0, 1, 0.1f);
	behaviour_net_mutation_sigma = get_numerical_option<float>(config_pt, "Population.behaviour_net_mutation_sigma", 0, 10, 0.2f);
	trait_genes_mutation_prob = get_numerical_option<float>(config_pt, "Population.trait_genes_mutation_prob", 0, 1, 0.1f);
	trait_genes_mutation_sigma = get_numerical_option<float>(config_pt, "Population.trait_genes_mutation_sigma", 0, 2, 0.01f);
}

// parse command line options excluding config file
void GeneticSimulation::Config::parse_command_line_options(const po::variables_map& vm)
{
	if (vm.count("run_mode")) {
		run_mode = vm["run_mode"].as<unsigned int>();
	}

	if (vm.count("simulation_threads")) {
		simulation_threads = vm["simulation_threads"].as<unsigned int>();
	}

#ifdef GPU_SUPPORT
	if (vm.count("planet_gpu")) {
		precompute_temperatures_gpu = vm["planet_gpu"].as<bool>();
	}
#endif

	if (vm.count("planet_cpu_threads")) {
		precompute_temperatures_cpu_threads = vm["planet_cpu_threads"].as<unsigned int>();
	}

	if (vm.count("benchmark_timesteps")) {
		simulation_benchmark_timesteps = vm["benchmark_timesteps"].as<unsigned int>();
	}

	if (vm.count("planet_benchmark_samples")) {
		planet_benchmark_samples = vm["planet_benchmark_samples"].as<unsigned int>();
	}
}

// convert a 3-byte hex string into a 32-bit color value
uint32_t GeneticSimulation::Config::parse_hex_color(const string& color_string)
{
	uint32_t color_value = 255;
	for (int i = 0; i < 3; i++) {
		color_value |= (stoi(color_string.substr(i * 2, 2), nullptr, 16) << ((3 - i) * 8));
	}
	return color_value;
}