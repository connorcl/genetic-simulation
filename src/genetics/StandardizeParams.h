#pragma once

namespace GeneticSimulation
{
	// parameters for standardizing a value (mean and standard deviation)
	struct StandardizeParams
	{
		// constructor which takes mean and standard deviation
		StandardizeParams(float mean, float sigma);

		// parameters
		float mean;
		float sigma;
	};
}