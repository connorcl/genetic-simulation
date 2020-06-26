#include "Planet.h"
#include "helper/benchmark_helper.h"
#include <cmath>
#include <vector>
#include <chrono>
#include <string>
#include <memory>
#ifdef GPU_SUPPORT
#include <amp.h>
#include <amp_math.h>
#endif
#include <algorithm>
#include <thread>
#include <boost/math/special_functions/sign.hpp>

using std::vector;
using std::chrono::steady_clock;
using std::chrono::microseconds;
using std::chrono::duration_cast;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::make_unique;
using std::min;
using std::max;
using std::thread;

using namespace GeneticSimulation;

// default constructor
GeneticSimulation::Planet::Planet() : initialized(false), timesteps(0) {}

// precompute temperatures
void GeneticSimulation::Planet::precompute_temperatures(const Config& config, bool benchmark)
{
	// set lookup table to correct size
	temperatures.resize(config.area_height * config.orbital_period);
	timesteps = config.orbital_period;

#ifdef GPU_SUPPORT
	// get whether to use GPU
	const bool use_cpu = !config.precompute_temperatures_gpu;
#else
	constexpr bool use_cpu = true;
#endif
	
	// precompute or benchmark on selected device
	if (use_cpu) {
		// determine number of threads to use
		auto num_threads = config.precompute_temperatures_cpu_threads == 0 ?
			thread::hardware_concurrency() :
			config.precompute_temperatures_cpu_threads;
		// benchmark or precompute once
		benchmark ? benchmark_temperature_computation_cpu(num_threads, config) : 
			precompute_temperatures_cpu(num_threads, config);
	}
#ifdef GPU_SUPPORT
	else {
		// benchmark or precompute once
		benchmark ? benchmark_temperature_computation_gpu(config) :
			precompute_temperatures_gpu(config);
	}
#endif

	// record initialization
	initialized = true;
}

// get temperature from lookup table
float GeneticSimulation::Planet::get_temperature(unsigned int y, unsigned int t) const
{
	// return -1 if temperatures have not been computed, otherwise return precomputed temperature
	return initialized ? temperatures[y * timesteps + (t % timesteps)] : -1.f;
}

// precompute temperatures using the CPU
void GeneticSimulation::Planet::precompute_temperatures_cpu(unsigned int worker_threads, const Config& config)
{
	// calculate number of timesteps per thread
	unsigned int timesteps_per_thread = timesteps / worker_threads + 1;
	// create vector for thread objects
	vector<unique_ptr<thread>> threads;
	// start threads
	for (unsigned int i = 0; i < worker_threads; i++) {
		threads.push_back(make_unique<thread>(
			[&, timesteps_per_thread, i] {
				precompute_temperatures_for_timestep_range_cpu(i * timesteps_per_thread,
					(i + 1) * timesteps_per_thread, config);
			}
		));
	}
	// join threads
	for (auto& t_ptr : threads) {
		t_ptr->join();
	}
}

// precompute temperatures on the CPU for the given timestep range
void GeneticSimulation::Planet::precompute_temperatures_for_timestep_range_cpu(unsigned int start_t, unsigned int end_t, const Config& config)
{
	// cap end_t
	end_t = min(end_t, config.orbital_period);

	// initialize vector for storing intermediate equatorial temperature results
	vector<double> equatorial_black_body_temperatures(end_t - start_t);

	// pi
	const double pi = 3.14159265358979323846;

	// reused loop variables
	double angle, pos_x, pos_y, squared_dist, black_body_temperature;

	// calculate equatorial black body temperatures for each timestep (angle in orbit)
	for (unsigned int t = start_t; t < end_t; t++)
	{
		// calculate orbital angle corresponding to timestep
		angle = (static_cast<double>(t) / static_cast<double>(config.orbital_period)) * 2 * pi;

		// calculate the x and y coordinates of the planet at this angle in the orbital ellipse
		pos_x = (config.orbit_radius_x * cos(angle) * cos(config.orbit_rotation)) -
			(config.orbit_radius_y * sin(angle) * sin(config.orbit_rotation)) +
			config.orbit_center_offset_x;
		pos_y = (config.orbit_radius_x * cos(angle) * sin(config.orbit_rotation)) +
			(config.orbit_radius_y * sin(angle) * cos(config.orbit_rotation)) +
			config.orbit_center_offset_y;
		// calculate squared distance from star (0, 0) based on these coordinates
		squared_dist = pos_x * pos_x + pos_y * pos_y;

		// calculate equivalent black body temperature based on this squared distance
		black_body_temperature = pow((config.star_luminosity * (1 - config.albedo)) /
			(16 * pi * squared_dist * 5.670373e-8), 0.25);

		// calculate an approximated equatorial temperature from the average black body temperature
		equatorial_black_body_temperatures[t - start_t] = black_body_temperature / cos(pi / 6.0);
	}

	// reused loop variables
	double latitude, angle_from_vernal_equinox, effective_axial_tilt, effective_latitude,
		height_to_latitude, effective_tilt_plane_dist, width_at_latitude, plane_dist_radius_ratio,
		extra_logitude, daylight_proportion, radiation_strength, base_temperature, moderated_temperature;

	// calculate final temperatures based on timestep (angle in orbit) and y position (latitude)
	for (unsigned int y = 0; y < config.area_height; y++)
	{
		// calculate latitude corresponding to y coordinate
		latitude = -(((static_cast<double>(y) / static_cast<double>(config.area_height - 1))
			* (90.f - -90.f)) - 90.f);

		for (unsigned int t = start_t; t < end_t; t++)
		{
			// calculate orbital angle corresponding to timestep
			angle = (static_cast<double>(t) / static_cast<double>(config.orbital_period)) * 2 * pi;

			// calculate effective axial tilt
			angle_from_vernal_equinox = angle + config.orbit_rotation;
			effective_axial_tilt = sin(angle_from_vernal_equinox) * config.axial_tilt;

			// calculate effective latitude based on effective axial tilt
			effective_latitude = latitude - effective_axial_tilt;

			// calculate the vertical height to the current latitude
			height_to_latitude = sin((latitude / 360.0) * 2 * pi) * config.radius;
			// calculate distance between axially tilted plane and plane
			// dividing day and night, travelling along latitude
			effective_tilt_plane_dist = tan((effective_axial_tilt / 360.0) * 2 * pi) * height_to_latitude;
			// calculate the width of the planet at the current latitude
			width_at_latitude = max(0., cos((latitude / 360.0) * 2 * pi) * config.radius);
			// calculate a safe ratio of the plane distance to the width at latitude
			plane_dist_radius_ratio = width_at_latitude == 0 ?
				boost::math::sign(effective_tilt_plane_dist) :
				effective_tilt_plane_dist / width_at_latitude;
			// calculate the extra longitude in or out of daylight
			extra_logitude = asin(max(-1.0, (min(1.0, plane_dist_radius_ratio))));
			// calculate the proportion of daylight hours at current latitude and effective tilt
			daylight_proportion = (pi + 2.0 * extra_logitude) / (2 * pi);

			// calculate solar radiation strength at current effective latitude
			radiation_strength = max(0., cos((effective_latitude / 360) * 2 * pi));

			// calculate base temperature
			base_temperature = equatorial_black_body_temperatures[t - start_t]
				* radiation_strength * (daylight_proportion * 2);

			// calculate moderated temperature to account for convection etc.
			moderated_temperature = ((base_temperature
				- (equatorial_black_body_temperatures[t - start_t] * config.temperature_moderation_bias))
				/ config.temperature_moderation_factor) + (equatorial_black_body_temperatures[t - start_t]
					* config.temperature_moderation_bias);

			// calculate final temperature which accounts for greenhouse effect
			temperatures[y * config.orbital_period + t] = moderated_temperature
				* pow((1 + 0.75 * config.atmosphere_optical_thickness), 0.25);
		}
	}
}

#ifdef GPU_SUPPORT
// precompute temperatures on the GPU
void GeneticSimulation::Planet::precompute_temperatures_gpu(const Config& config)
{
	// use concurrency namespace for C++ AMP
	using namespace concurrency;

	// pi
	const float pi = 3.14159f;

	// initialize vector for storing intermediate equatorial temperature results
	vector<float> equatorial_black_body_temperatures(config.orbital_period);
	// initialize array view for this vector
	array_view<float, 1> equatorial_black_body_temperatures_av(config.orbital_period,
		equatorial_black_body_temperatures);
	// do not transfer 0s to GPU
	equatorial_black_body_temperatures_av.discard_data();

	// create array view for temperatures lookup table
	array_view<float, 2> temperatures_av(config.area_height, config.orbital_period, temperatures);
	// do not transfer 0s to GPU
	temperatures_av.discard_data();

	// save config values in local variables so AMP can use them
	const unsigned int area_height = config.area_height;
	const float latitude_range = config.latitude_range;
	const unsigned int orbital_period = config.orbital_period;
	const float orbit_center_offset_x = config.orbit_center_offset_x;
	const float orbit_center_offset_y = config.orbit_center_offset_y;
	const float orbit_radius_x = config.orbit_radius_x;
	const float orbit_radius_y = config.orbit_radius_y;
	const float orbit_rotation = config.orbit_rotation;
	const float star_luminosity = config.star_luminosity;
	const float albedo = config.albedo;
	const float axial_tilt = config.axial_tilt;
	const float radius = config.radius;
	const float atmosphere_optical_thickness = config.atmosphere_optical_thickness;
	const float temperature_moderation_factor = config.temperature_moderation_factor;
	const float temperature_moderation_bias = config.temperature_moderation_bias;

	// kernel to calculate equatorial black body temperatures for each time step
	parallel_for_each(
		equatorial_black_body_temperatures_av.extent,
		[=](index<1> idx) restrict(amp) {
			// calculate orbital angle corresponding to timestep
			float angle = (static_cast<float>(idx[0]) / static_cast<float>(orbital_period)) * 2 * pi;

			// calculate the x and y coordinates of the planet at this angle in the orbital ellipse
			float pos_x = (orbit_radius_x * fast_math::cos(angle) * fast_math::cos(orbit_rotation)) -
				(orbit_radius_y * fast_math::sin(angle) * fast_math::sin(orbit_rotation)) +
				orbit_center_offset_x;
			float pos_y = (orbit_radius_x * fast_math::cos(angle) * fast_math::sin(orbit_rotation)) +
				(orbit_radius_y * fast_math::sin(angle) * fast_math::cos(orbit_rotation)) +
				orbit_center_offset_y;
			// calculate squared distance from star (0, 0) based on these coordinates
			float squared_dist = pos_x * pos_x + pos_y * pos_y;

			// calculate equivalent black body temperature based on this squared distance
			float black_body_temperature = fast_math::pow((star_luminosity * (1 - albedo)) /
				(16 * pi * squared_dist * 5.670373e-8), 0.25);

			// calculate an approximated equatorial temperature from the average black body temperature
			equatorial_black_body_temperatures_av[idx] = black_body_temperature / fast_math::cos(pi / 6.0);
		}
	);

	// kernel to calculate final temperatures for each time step and latitude
	parallel_for_each(
		temperatures_av.extent,
		[=](index<2> idx) restrict(amp)
		{
			// calculate latitude corresponding to y coordinate
			float latitude = -(((static_cast<float>(idx[0]) / static_cast<float>(area_height - 1))
				* (90.f - -90.f)) - 90.f);

			// calculate orbital angle corresponding to timestep
			float angle = (static_cast<float>(idx[1]) / static_cast<float>(orbital_period)) * 2 * pi;

			// calculate effective axial tilt
			float angle_from_vernal_equinox = angle + orbit_rotation;
			float effective_axial_tilt = fast_math::sin(angle_from_vernal_equinox) * axial_tilt;

			// calculate effective latitude based on effective axial tilt
			float effective_latitude = latitude - effective_axial_tilt;

			// calculate the vertical height to the current latitude
			float height_to_latitude = fast_math::sin((latitude / 360.0) * 2 * pi) * radius;
			// calculate distance between axially tilted plane and plane
			// dividing day and night, travelling along latitude
			float effective_tilt_plane_dist = fast_math::tan((effective_axial_tilt / 360.0) * 2 * pi) * height_to_latitude;
			// calculate the width of the planet at the current latitude
			float width_at_latitude = max(0, fast_math::cos((latitude / 360.0) * 2 * pi) * radius);
			// calculate a safe ratio of the plane distance to the width at latitude
			float plane_dist_radius_ratio = width_at_latitude == 0 ?
				(0 < effective_tilt_plane_dist) - (effective_tilt_plane_dist < 0) :
				effective_tilt_plane_dist / width_at_latitude;
			// calculate the extra longitude in or out of daylight
			float extra_logitude = fast_math::asin(max(-1.0, (min(1.0, plane_dist_radius_ratio))));
			// calculate the proportion of daylight hours at current latitude and effective tilt
			float daylight_proportion = (pi + 2.0 * extra_logitude) / (2 * pi);

			// calculate solar radiation strength at current effective latitude
			float radiation_strength = max(0, fast_math::cos((effective_latitude / 360) * 2 * pi));

			// save equatorial black body temperature
			float equatorial_black_body_temperature = equatorial_black_body_temperatures_av[idx[1]];

			// calculate base temperature
			float base_temperature = equatorial_black_body_temperature
				* radiation_strength * (daylight_proportion * 2);

			// calculate moderated temperature to account for convection etc.
			float moderated_temperature = ((base_temperature
				- (equatorial_black_body_temperature * temperature_moderation_bias))
				/ temperature_moderation_factor) + (equatorial_black_body_temperature
					* temperature_moderation_bias);

			// calculate final temperature which accounts for greenhouse effect
			temperatures_av[idx] = moderated_temperature
				* fast_math::pow((1 + 0.75 * atmosphere_optical_thickness), 0.25);
		}
	);

	// transfer temperature data from GPU
	temperatures_av.synchronize();
}
#endif

// benchmark precomputation on the CPU
void GeneticSimulation::Planet::benchmark_temperature_computation_cpu(unsigned int worker_threads, const Config& config)
{
	// time points for benchmarking
	steady_clock::time_point start, end;

	// allocate space to store results
	vector<unsigned long long> times(config.planet_benchmark_samples);

	// precompute temperatures and record results
	for (unsigned int i = 0; i < config.planet_benchmark_samples; i++) {
		start = steady_clock::now();
		precompute_temperatures_cpu(worker_threads, config);
		end = steady_clock::now();
		times[i] = duration_cast<microseconds>(end - start).count();
	}

	// output results to file
	string filename = "planet_benchmark_cpu_" + to_string(worker_threads) + "_threads.csv";
	string header = "time_microseconds_" + to_string(worker_threads) + " _threads";
	write_benchmark_results(times, header, filename, config.results_path);
}

#ifdef GPU_SUPPORT
// benchmark precomputation on the GPU
void GeneticSimulation::Planet::benchmark_temperature_computation_gpu(const Config& config)
{
	// time points for benchmarking
	steady_clock::time_point start, end;

	// allocate space to store results
	vector<unsigned long long> times(config.planet_benchmark_samples);

	// precompute temperatures and record results
	for (unsigned int i = 0; i < config.planet_benchmark_samples; i++) {
		start = steady_clock::now();
		precompute_temperatures_gpu(config);
		end = steady_clock::now();
		times[i] = duration_cast<microseconds>(end - start).count();
	}

	// output results to file
	string filename = "planet_benchmark_gpu.csv";
	string header = "time_microseconds_gpu";
	write_benchmark_results(times, header, filename, config.results_path);
}
#endif