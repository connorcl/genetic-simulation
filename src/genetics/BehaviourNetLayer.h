#pragma once

#include <vector>
#include <random>

namespace GeneticSimulation
{
	// A fully connected neural network layer plus a sigmoid/tanh activation function layer,
	// which forms part of a neural network used for determining an organism's behaviour
	class BehaviourNetLayer
	{
	public:

		// constructor which takes numbers of inputs and units
		// and whether to use the sigmoid activation function 
		BehaviourNetLayer(unsigned int inputs, unsigned int units, bool sigmoid = false);

		// forward pass operator
		const std::vector<float>& operator()(const std::vector<float>& input);

		// generate random weights
		void init_random(float range, float range_bias, std::default_random_engine& rng);

		// initialize weight vector by combining parents and mutating
		void init_from(const BehaviourNetLayer& parent1,
			const BehaviourNetLayer& parent2, float mutation_prob,
			float mutation_sigma, std::default_random_engine& rng);

		// initialize weight vector by copying parent and mutating
		void init_from(const BehaviourNetLayer& parent, float mutation_prob,
			float mutation_sigma, std::default_random_engine& rng);

		// transfer information from another weight vector
		void transfer_from(const BehaviourNetLayer& donor, float donor_weighting);

	private:

		// apply tanh function to activations
		void tanh_activation();

		// apply sigmoid function to activations
		void sigmoid_activation();

		// numbers of inputs and units
		const unsigned int inputs, units;
		// whether to use sigmoid activation function over default tanh
		const bool sigmoid;
		// weights
		std::vector<float> weights;
		// activations (outputs)
		std::vector<float> activations;
	};
}