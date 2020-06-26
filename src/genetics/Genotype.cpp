#include "Genotype.h"
#include "genetic_helper.h"
#include <mutex>

using std::default_random_engine;
using std::vector;
using std::scoped_lock;

using namespace GeneticSimulation;

// constructor
GeneticSimulation::Genotype::Genotype(unsigned int sensory_values, 
	unsigned int behaviour_net_nh1, unsigned int behaviour_net_nh2, unsigned int decision_values) :
	// initialize behaviour network and trait genes
	behaviour_net(sensory_values, behaviour_net_nh1, behaviour_net_nh2, decision_values),
	trait_genes(15) {}

// copy constructor
GeneticSimulation::Genotype::Genotype(const Genotype& rhs) :
	behaviour_net(rhs.behaviour_net),
	trait_genes(rhs.trait_genes) {}

// randomly initialize genotype
void GeneticSimulation::Genotype::init_random(float behaviour_net_range, 
	float behaviour_net_range_bias, default_random_engine& rng)
{
	// randomly initialize behaviour network
	behaviour_net.init_random(behaviour_net_range, behaviour_net_range_bias, rng);
	// randomize trait genes with standard normal values
	randomize_normal(trait_genes, 0.f, 1.f, rng);
}

// initialize genotype from parents
void GeneticSimulation::Genotype::init_from(const Genotype& parent1, 
	const Genotype& parent2, float behaviour_net_mutation_prob, 
	float behaviour_net_mutation_sigma, float trait_genes_mutation_prob, 
	float trait_genes_mutation_sigma, std::default_random_engine& rng)
{
	// combine weights in parent behaviour networks and mutate
	behaviour_net.init_from(parent1.behaviour_net, parent2.behaviour_net,
		behaviour_net_mutation_prob, behaviour_net_mutation_sigma, rng);
	// combine parent trait genes and mutate 
	combine_and_mutate_random(trait_genes, parent1.trait_genes, parent2.trait_genes,
		trait_genes_mutation_prob, trait_genes_mutation_sigma, rng);
}

// initialize genotype from single parent
void GeneticSimulation::Genotype::init_from(const Genotype& parent, 
	float behaviour_net_mutation_prob, float behaviour_net_mutation_sigma,
	float trait_genes_mutation_prob, float trait_genes_mutation_sigma, 
	default_random_engine& rng)
{
	// copy weights from parent behaviour network and mutate
	behaviour_net.init_from(parent.behaviour_net, behaviour_net_mutation_prob, 
		behaviour_net_mutation_sigma, rng);
	// copy parent trait genes
	trait_genes = parent.trait_genes;
	// mutate trait genes
	mutate(trait_genes, trait_genes_mutation_prob, trait_genes_mutation_sigma, rng);
}

// transfer information from donor genotype
void GeneticSimulation::Genotype::transfer_from(Genotype& donor, float donor_weighting)
{
	// safely lock donor and recipient mutexes
	scoped_lock lock(mx, donor.mx);
	// transfer information from donor behaviour network
	behaviour_net.transfer_from(donor.behaviour_net, donor_weighting);
	// transfer information from donor trait genes
	combine(trait_genes, donor.trait_genes, trait_genes, donor_weighting);
}

// express behaviour based on behaviour net and sensory data
const vector<float>& GeneticSimulation::Genotype::express_behaviour(const vector<float>& sensory_data)
{
	// pass sensory data through the behaviour network and return decision
	return behaviour_net(sensory_data);
}

// express physical traits based on genes and record in phenotype
void GeneticSimulation::Genotype::express_traits(Phenotype& phenotype)
{
	// set area of influence based on average of first four trait genes
	phenotype.set_area_of_influence(calculate_trait(0, 4));
	// set speed based on average of negative of second four trait genes,
	// overlapping with area of influence
	phenotype.set_speed(calculate_trait(3, 4, true));
	// set health rate based on average of negative of 
	// third three trait genes, overlapping with speed
	phenotype.set_health_rate(calculate_trait(6, 3, true));
	// set ideal temperature based on next three trait genes
	phenotype.set_ideal_temp(calculate_trait(9, 3));
	// set temperature range based on last three trait genes
	phenotype.set_temp_range(calculate_trait(12, 3));
}

// calculate the value of a trait by combining trait genes
float GeneticSimulation::Genotype::calculate_trait(unsigned int start_i, unsigned int n, bool negate)
{
	// negate trait value if specified
	float multiplier = negate ? -1.f : 1.f;
	// start value at 0
	float val = 0;
	// average n trait genes starting from start index
	for (unsigned int i = 0; i < n; i++) {
		val += trait_genes[start_i + i];
	}
	// return trait value
	return multiplier * val / static_cast<float>(n);
}