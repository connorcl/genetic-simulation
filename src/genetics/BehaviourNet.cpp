#include "BehaviourNet.h"

using std::vector;
using std::default_random_engine;

using namespace GeneticSimulation;

// constructor which takes architecture details
GeneticSimulation::BehaviourNet::BehaviourNet(unsigned int ni, unsigned int nh1, unsigned int nh2, unsigned int no) :
	layer1(ni, nh1), layer2(nh1, nh2), output_layer(nh2, no) {}

// forward pass operator
const vector<float>& GeneticSimulation::BehaviourNet::operator()(const vector<float>& input)
{
	return output_layer(layer2(layer1(input)));
}

// randomly initialize layer weights
void GeneticSimulation::BehaviourNet::init_random(float weights_range, float range_bias, default_random_engine& rng)
{
	layer1.init_random(weights_range, range_bias, rng);
	layer2.init_random(weights_range, range_bias, rng);
	output_layer.init_random(weights_range, range_bias, rng);
}

// initialize layer weights from parents
void GeneticSimulation::BehaviourNet::init_from(const BehaviourNet& parent1, 
	const BehaviourNet& parent2, float mutation_prob, float mutation_sigma, 
	default_random_engine& rng)
{
	layer1.init_from(parent1.layer1, parent2.layer1, 
		mutation_prob, mutation_sigma, rng);
	layer2.init_from(parent1.layer2, parent2.layer2, 
		mutation_prob, mutation_sigma, rng);
	output_layer.init_from(parent1.output_layer, parent2.output_layer, 
		mutation_prob, mutation_sigma, rng);
}

// initialize layer weights from single parent
void GeneticSimulation::BehaviourNet::init_from(const BehaviourNet& parent, 
	float mutation_prob, float mutation_sigma, default_random_engine& rng)
{
	layer1.init_from(parent.layer1, mutation_prob, mutation_sigma, rng);
	layer2.init_from(parent.layer2, mutation_prob, mutation_sigma, rng);
	output_layer.init_from(parent.output_layer, mutation_prob, mutation_sigma, rng);
}

// transfer information from donor
void GeneticSimulation::BehaviourNet::transfer_from(const BehaviourNet& donor, float donor_weighting)
{
	layer1.transfer_from(donor.layer1, donor_weighting);
	layer2.transfer_from(donor.layer2, donor_weighting);
	output_layer.transfer_from(donor.output_layer, donor_weighting);
}