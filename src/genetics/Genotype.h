#pragma once

#include "BehaviourNet.h"
#include "Phenotype.h"
#include <random>
#include <vector>
#include <mutex>

namespace GeneticSimulation
{
	// The genetic information of an organism which is
	// expressed to produce behaviour and physical traits
	class Genotype
	{
	public:

		// constructor
		Genotype(unsigned int sensory_values, unsigned int behaviour_net_nh1, 
			unsigned int behaviour_net_nh2, unsigned int decision_values);

		// copy constructor
		Genotype(const Genotype& rhs);

		// randomly initialize genotype
		void init_random(float behaviour_net_range, float behaviour_net_range_bias, 
			std::default_random_engine& rng);

		// initialize genotype from parents
		void init_from(const Genotype& parent1, const Genotype& parent2,
			float behaviour_net_mutation_prob, float behaviour_net_mutation_sigma, 
			float trait_genes_mutation_prob, float trait_genes_mutation_sigma,
			std::default_random_engine& rng);

		// initialize genotype from single parent
		void init_from(const Genotype& parent, float behaviour_net_mutation_prob, 
			float behaviour_net_mutation_sigma, float trait_genes_mutation_prob, 
			float trait_genes_mutation_sigma, std::default_random_engine& rng);

		// transfer information from donor genotype
		void transfer_from(Genotype& donor, float donor_weighting);

		// express behaviour based on behaviour net and sensory data
		const std::vector<float>& express_behaviour(const std::vector<float>& sensory_data);

		// express physical traits based on trait genes and record in phenotype
		void express_traits(Phenotype& phenotype);

	private:

		// calculate the value of a trait by combining trait genes
		float calculate_trait(unsigned int start_i, unsigned int n, bool negate = false);

		// mutex which protects genes during gene transfer
		std::mutex mx;
		// neural network which codes for behaviour
		BehaviourNet behaviour_net;
		// genetic sequence which codes for physical traits
		std::vector<float> trait_genes;
	};
}