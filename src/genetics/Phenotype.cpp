#include "Phenotype.h"

using namespace GeneticSimulation;

// constructor which takes standardization parameters for each trait
GeneticSimulation::Phenotype::Phenotype(StandardizeParams area_params, 
	StandardizeParams speed_params, StandardizeParams rate_params, 
	StandardizeParams ideal_temp_params, StandardizeParams temp_range_params) :
	// initialize each physical trait
	area_of_influence(0, area_params), speed(0, speed_params), health_rate(0, rate_params),
	ideal_temp(0, ideal_temp_params), temp_range(0, temp_range_params) {}

// get area of influence
float GeneticSimulation::Phenotype::get_area_of_influence() const
{
	return area_of_influence.get_value();
}

// get speed
float GeneticSimulation::Phenotype::get_speed() const
{
	return speed.get_value();
}

// get health rate
float GeneticSimulation::Phenotype::get_health_rate() const
{
	return health_rate.get_value();
}

// get ideal temperature
float GeneticSimulation::Phenotype::get_ideal_temp() const
{
	return ideal_temp.get_value();
}

// get temperature range
float GeneticSimulation::Phenotype::get_temp_range() const
{
	return temp_range.get_value();
}

// set area of influence
void GeneticSimulation::Phenotype::set_area_of_influence(float area_standardized)
{
	area_of_influence.set_value_from_standardized(area_standardized);
}

// set speed
void GeneticSimulation::Phenotype::set_speed(float speed_standardized)
{
	speed.set_value_from_standardized(speed_standardized);
}

// set health rate
void GeneticSimulation::Phenotype::set_health_rate(float rate_standardized)
{
	health_rate.set_value_from_standardized(rate_standardized);
}

// set ideal temperature
void GeneticSimulation::Phenotype::set_ideal_temp(float temp_standardized)
{
	ideal_temp.set_value_from_standardized(temp_standardized);
}

// set temperature range
void GeneticSimulation::Phenotype::set_temp_range(float temp_standardized)
{
	temp_range.set_value_from_standardized(temp_standardized);
}