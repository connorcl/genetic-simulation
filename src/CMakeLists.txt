# genetic_simulation

# minimum CMake version requirement
cmake_minimum_required(VERSION 3.10)

# project name
project(genetic_simulation)

# find Boost libraries
find_package(Boost 1.62 REQUIRED COMPONENTS program_options filesystem thread)
# find SFML libraries
find_package(SFML REQUIRED COMPONENTS graphics system)

# set Boost options
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# add subdirectories for each sub-component
add_subdirectory(helper)
add_subdirectory(engine)
add_subdirectory(genetics)

# add source files
add_executable(genetic_simulation
	Config.cpp Config.h
	ConsumableResource.cpp ConsumableResource.h
	ConsumableResourcePool.cpp ConsumableResourcePool.h
	Organism.cpp Organism.h
	Planet.cpp Planet.h
	Population.cpp Population.h
	SensoryData.cpp SensoryData.h
	Simulation.cpp Simulation.h
	main.cpp)

# link with each sub-component
target_link_libraries(genetic_simulation PUBLIC helper engine genetics)
# link with Boost
target_link_libraries(genetic_simulation PUBLIC Boost::program_options Boost::filesystem Boost::thread)
# link with SFML
target_link_libraries(genetic_simulation PUBLIC sfml-graphics sfml-system)

# require C++17 support
set_property(TARGET genetic_simulation PROPERTY CXX_STANDARD 17)
# enable whole-program/link-time optimization
set_property(TARGET genetic_simulation PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)