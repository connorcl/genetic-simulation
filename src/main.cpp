#include "Config.h"
#include "Simulation.h"

using namespace GeneticSimulation;

int main(int argc, char* argv[])
{
	// initialize configuration options
	Config config;
	config.init(argc, argv);
	// initialize simulation
	Simulation simulation(config);
	simulation.init();
	// run simulation
	simulation.run();

	return 0;
}