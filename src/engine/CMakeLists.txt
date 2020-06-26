# engine

# add source files
add_library(engine
	SimulationArea.cpp SimulationArea.h
	SimulationObject.cpp SimulationObject.h
	SimulationObjectPool.h)

# link with SFML
target_link_libraries(engine PUBLIC sfml-graphics sfml-system)

# require C++17 support
set_property(TARGET engine PROPERTY CXX_STANDARD 17)
# enable whole-program/link-time optimization
set_property(TARGET engine PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)