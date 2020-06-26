# helper

# add source files
add_library(helper
	benchmark_helper.cpp benchmark_helper.h
	color.cpp color.h
	SignalLink.cpp SignalLink.h
	numbers.cpp numbers.h
	ConcurrentQueue.h
	platform.h)

# link with Boost
target_link_libraries(helper PRIVATE Boost::filesystem Boost::thread)
# link with SFML
target_link_libraries(helper PUBLIC sfml-graphics)

# require C++17 support
set_property(TARGET helper PROPERTY CXX_STANDARD 17)
# enable whole-program/link-time optimization
set_property(TARGET helper PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)