#include "BehaviourNetLayer.h"
#include "genetic_helper.h"
#include <cmath>
#include <algorithm>

using std::vector;
using std::default_random_engine;
using std::max;

using namespace GeneticSimulation;

// constructor which takes numbers of inputs and units
// and whether to use the sigmoid activation function 
GeneticSimulation::BehaviourNetLayer::BehaviourNetLayer(unsigned int inputs, 
	unsigned int units, bool sigmoid) :
	inputs(inputs), units(units), sigmoid(sigmoid),
	weights(inputs * units), activations(units) {}

// forward pass operator
const vector<float>& GeneticSimulation::BehaviourNetLayer::operator()(const vector<float>& input)
{
	// zero output
	for (auto& i : activations) {
		i = 0;
	}

	// compute product of input vector and weight matrix

	// for each of left columns / right rows
	for (unsigned int k = 0; k < inputs; k++) {
		// for each of right columns
		for (unsigned int j = 0; j < units; j++) {
			// output[i][j] += left[i][k] * right[k][j]
			activations[j] += input[k] * weights[k * units + j];
		}
	}

	// apply activation function
	sigmoid ? sigmoid_activation() : tanh_activation();

	// return reference to const activations
	return activations;
}

// generate random weights
void GeneticSimulation::BehaviourNetLayer::init_random(float range, 
	float range_bias, default_random_engine& rng)
{
	// cap range at minimum 0.1
	range = max(0.1f, range);
	// cap range bias minimum at 1
	range_bias = max(1.f, range_bias);
	// generate random uniform weights
	randomize_uniform(weights, -range / range_bias, range, rng);
}

// initialize weight vectors by combining parents and mutating
void GeneticSimulation::BehaviourNetLayer::init_from(const BehaviourNetLayer& parent1, 
	const BehaviourNetLayer& parent2, float mutation_prob, 
	float mutation_sigma, default_random_engine& rng)
{
	// combine and mutate parent weights
	combine_and_mutate_random(weights, parent1.weights, parent2.weights, 
		mutation_prob, mutation_sigma, rng);
}

// initialize weight vector by copying parent and mutating
void GeneticSimulation::BehaviourNetLayer::init_from(const BehaviourNetLayer& parent, 
	float mutation_prob, float mutation_sigma, default_random_engine& rng)
{
	// copy parent weights
	weights = parent.weights;
	// mutate weights
	mutate(weights, mutation_prob, mutation_sigma, rng);
}

// transfer information from a donor
void GeneticSimulation::BehaviourNetLayer::transfer_from(const BehaviourNetLayer& donor, 
	float donor_weighting)
{
	// combine weights with donor weights
	combine(weights, donor.weights, weights, donor_weighting);
}

// apply sigmoid function to activations
void GeneticSimulation::BehaviourNetLayer::sigmoid_activation()
{
	for (auto& i : activations) {
		i = 1.f / (1.f + exp(-i));
	}
}

// apply tanh function to activations
void GeneticSimulation::BehaviourNetLayer::tanh_activation()
{
	for (auto& i : activations) {
		i = tanh(i);
	}
}