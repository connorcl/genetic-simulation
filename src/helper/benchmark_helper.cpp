#include "benchmark_helper.h"
#include <iostream>
#include <boost/filesystem.hpp>

using std::cout;
using std::cerr;
using std::ios;

// write benchmark results to file
void GeneticSimulation::write_benchmark_results(const std::vector<unsigned long long>& times, 
	const std::string& header, const std::string& filename, const std::string& path)
{
	// alias for boost filesystem namespace
	namespace fs = boost::filesystem;

	// generate path for results file
	fs::path results_file_path(path);
	results_file_path /= filename;

	// output name of results file
	cout << "Writing benchmark results to " << results_file_path.string() << "\n";

	// attempt to write results
	try {
		// open file
		fs::ofstream results_file(results_file_path, ios::trunc);
		// print error and return if opening file failed
		if (!results_file) {
			cerr << "Writing results file failed: Check that the path exists and may be written to\n";
			return;
		}
		// write header
		results_file << header << "\n";
		// write times
		for (auto time : times) {
			results_file << time << "\n";
		}
		// close file
		results_file.close();
	}
	catch (const fs::filesystem_error& e) {
		// if writing results fails, log error
		cerr << "Writing results file failed: " << e.what() << "\n";
	}
}