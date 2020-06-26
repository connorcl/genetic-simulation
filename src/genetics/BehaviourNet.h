#pragma once

#include "BehaviourNetLayer.h"
#include <vector>
#include <random>

namespace GeneticSimulation
{
	// A simple feedforward artificial neural network with two
	// hidden layers which determines an organism's behaviour
	class BehaviourNet
	{
	public:

		// constructor which takes architecture details
		BehaviourNet(unsigned int ni, unsigned int nh1, unsigned int nh2, unsigned int no);

		// forward pass operator
		const std::vector<float>& operator()(const std::vector<float>& input);

		// randomly initialize layer weights
		void init_random(float weights_range, float range_bias, std::default_random_engine& rng);

		// initialize layer weights from parents
		void init_from(const BehaviourNet& parent1, const BehaviourNet& parent2,
			float mutation_prob, float mutation_sigma, std::default_random_engine& rng);

		// initialize layer weights from single parent
		void init_from(const BehaviourNet& parent, float mutation_prob,
			float mutation_sigma, std::default_random_engine& rng);

		// transfer information from donor
		void transfer_from(const BehaviourNet& donor, float donor_weighting);

	private:

		// layers
		BehaviourNetLayer layer1;
		BehaviourNetLayer layer2;
		BehaviourNetLayer output_layer;
	};
}