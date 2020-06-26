#include "PhysicalTrait.h"

// constructor which takes a value and the parameters used to standardize values
GeneticSimulation::PhysicalTrait::PhysicalTrait(float value, StandardizeParams params) : 
	value(value), params(params) {}

// get the value of the trait
float GeneticSimulation::PhysicalTrait::get_value() const
{
	return value;
}

// set the value of the trait
void GeneticSimulation::PhysicalTrait::set_value(float val)
{
	value = val;
}

// set the value of the trait, automatically converting from standardized form
void GeneticSimulation::PhysicalTrait::set_value_from_standardized(float standardized_val)
{
	value = standardized_val * params.sigma + params.mean;
}