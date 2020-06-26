#pragma once

#include "StandardizeParams.h"
#include "PhysicalTrait.h"

namespace GeneticSimulation
{
	// The physical traits of an organism which are coded for in its genotype
	class Phenotype
	{
	public:

		// constructor which takes standardization parameters for each trait
		Phenotype(StandardizeParams area_params, StandardizeParams speed_params,
			StandardizeParams rate_params, StandardizeParams ideal_temp_params,
			StandardizeParams temp_range_params);

		// getters for each trait value
		float get_area_of_influence() const;
		float get_speed() const;
		float get_health_rate() const;
		float get_ideal_temp() const;
		float get_temp_range() const;

		// setters for each trait value which convert from standardized form
		void set_area_of_influence(float area_standardized);
		void set_speed(float speed_standardized);
		void set_health_rate(float rate_standardized);
		void set_ideal_temp(float temp_standardized);
		void set_temp_range(float temp_standardized);

	private:

		// traits
		PhysicalTrait area_of_influence;
		PhysicalTrait speed;
		PhysicalTrait health_rate;
		PhysicalTrait ideal_temp;
		PhysicalTrait temp_range;
	};
}