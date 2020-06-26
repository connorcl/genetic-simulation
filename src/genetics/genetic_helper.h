#pragma once

#include <vector>
#include <random>

namespace GeneticSimulation
{
	// fill a vector with random normal values
	template<typename T>
	void randomize_normal(std::vector<T>& vec, T mean, T sigma, std::default_random_engine& rng)
	{
		// create normal distribution
		std::normal_distribution<T> dist_norm(mean, sigma);
		// set each element to random value
		for (auto& i : vec) {
			i = dist_norm(rng);
		}
	}

	// fill a vector with random uniform values
	template<typename T>
	void randomize_uniform(std::vector<T>& vec, T min_val, T max_val, std::default_random_engine& rng)
	{
		// create uniform distribution
		std::uniform_real_distribution<T> dist_unif(min_val, max_val);
		// set each element to random value
		for (auto& i : vec) {
			i = dist_unif(rng);
		}
	}

	// combine two vectors to produce a weighted average
	template<typename T>
	void combine(std::vector<T>& child, const std::vector<T>& parent1,
		const std::vector<T>& parent2, T parent1_weighting)
	{
		// for each element
		for (unsigned int i = 0; i < child.size(); i++) {
			// child element is weighted average of parent elements
			child[i] = (parent1_weighting * parent1[i]) +
				((1.f - parent1_weighting) * parent2[i]);
		}
	}

	// randomly mutate the elements of a vector
	template<typename T>
	void mutate(std::vector<T>& vec, T mutation_prob,
		T mutation_sigma, std::default_random_engine& rng)
	{
		// uniform distribution from 0 to 1 for deciding whether to mutate
		std::uniform_real_distribution<T> dist_mutate(0, 1);
		// normal distribution for deciding mutation amount
		std::normal_distribution<T> dist_mutation_amount(0, mutation_sigma);

		// for each element
		for (unsigned int i = 0; i < vec.size(); i++) {
			// decide whether to mutate element
			if (dist_mutate(rng) <= mutation_prob) {
				// mutate child weight by normally distributed random amount
				vec[i] += dist_mutation_amount(rng);
			}
		}
	}

	// set a vector's elements by combining two parent vectors and mutating
	template<typename T>
	void combine_and_mutate_random(std::vector<T>& child,
		const std::vector<T>& parent1, const std::vector<T>& parent2,
		T mutation_prob, T mutation_sigma, std::default_random_engine& rng)
	{
		// uniform distribution from 0 to 1 for weighting parents
		std::uniform_real_distribution<T> dist_parent_weighting(0, 1);
		// combine parents
		combine(child, parent1, parent2, dist_parent_weighting(rng));
		// mutate child
		mutate(child, mutation_prob, mutation_sigma, rng);
	}
}