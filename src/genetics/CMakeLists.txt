# genetics

# add source files
add_library(genetics
	BehaviourNet.cpp BehaviourNet.h
	BehaviourNetLayer.cpp BehaviourNetLayer.h
	StandardizeParams.cpp StandardizeParams.h
	PhysicalTrait.cpp PhysicalTrait.h
	Phenotype.cpp Phenotype.h
	Genotype.cpp Genotype.h
    genetic_helper.h)

# require C++17 support
set_property(TARGET genetics PROPERTY CXX_STANDARD 17)
# enable whole-program/link-time optimization
set_property(TARGET genetics PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)