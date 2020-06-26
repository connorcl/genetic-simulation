#pragma once

#include <vector>
#include <string>

namespace GeneticSimulation
{
	// write benchmark results to file
	void write_benchmark_results(const std::vector<unsigned long long>& times,
		const std::string& header, const std::string& filename, const std::string& path);
}