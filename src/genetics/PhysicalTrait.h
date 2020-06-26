#pragma once

#include "StandardizeParams.h"

namespace GeneticSimulation
{
	// A physical trait which forms part of an organism's phenotype
	class PhysicalTrait
	{
	public:

		// constructor which takes a value and the parameters used to standardize values
		PhysicalTrait(float value, StandardizeParams params);

		// get the value of the trait
		float get_value() const;

		// set the value of the trait
		void set_value(float val);

		// set the value of the trait, automatically converting from standardized form
		void set_value_from_standardized(float standardized_val);

	private:

		// value of trait
		float value;
		// parameters used to standardize the value
		const StandardizeParams params;
	};
}