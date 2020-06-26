#include "SensoryData.h"
#include "helper/numbers.h"

// constructor
GeneticSimulation::SensoryData::SensoryData() :
	data(7, 0) {}

// gets hunger value
float GeneticSimulation::SensoryData::get_hunger()
{
	return hunger() / 2.f + 0.5f;
}

// gets thirst value
float GeneticSimulation::SensoryData::get_thirst()
{
	return thirst() / 2.f + 0.5f;
}

// gets temperature damage value
float GeneticSimulation::SensoryData::get_temperature_damage()
{
	return temperature_damage() / 2.f + 0.5f;
}

// gets heading to nearest food
float GeneticSimulation::SensoryData::get_food_heading()
{
	return food_heading() * pi;
}

// gets heading to nearest water
float GeneticSimulation::SensoryData::get_water_heading()
{
	return water_heading() * pi;
}

// gets direction to best temperature
float GeneticSimulation::SensoryData::get_temperature_heading()
{
	return temperature_heading() * pi;
}

// gets memory item
float GeneticSimulation::SensoryData::get_memory()
{
	return memory();
}

// returns a reference to the vector of scaled sensory values
const std::vector<float>& GeneticSimulation::SensoryData::get_data()
{
	return data;
}

// sets scaled hunger value based on nutrition
void GeneticSimulation::SensoryData::set_hunger(unsigned int nutrition)
{
	hunger() = ((1 - static_cast<float>(nutrition) / 1e6f) - 0.5f) * 2.f;
}

// sets scaled thirst value based on hydration
void GeneticSimulation::SensoryData::set_thirst(unsigned int hydration)
{
	thirst() = ((1 - static_cast<float>(hydration) / 1e6f) - 0.5f) * 2.f;
}

// set scaled temperature damage value based on integrity
void GeneticSimulation::SensoryData::set_temperature_damage(unsigned int integrity)
{
	temperature_damage() = ((1 - static_cast<float>(integrity) / 1e6f) - 0.5f) * 2.f;
}

// set scaled food heading
void GeneticSimulation::SensoryData::set_food_heading(float heading)
{
	food_heading() = heading / pi;
}

// set scaled water heading
void GeneticSimulation::SensoryData::set_water_heading(float heading)
{
	water_heading() = heading / pi;
}

// set scaled temperature heading
void GeneticSimulation::SensoryData::set_temperature_heading(float heading)
{
	temperature_heading() = heading / pi;
}

// set memory item
void GeneticSimulation::SensoryData::set_memory(float memory_item)
{
	memory() = memory_item;
}

// return reference to hunger data element
inline float& GeneticSimulation::SensoryData::hunger()
{
	return data[0];
}

// return reference to thirst data element
inline float& GeneticSimulation::SensoryData::thirst()
{
	return data[1];
}

// return reference to temperature damage data element
inline float& GeneticSimulation::SensoryData::temperature_damage()
{
	return data[2];
}

// return reference to food heading data element
inline float& GeneticSimulation::SensoryData::food_heading()
{
	return data[3];
}

// return reference to water heading data element
inline float& GeneticSimulation::SensoryData::water_heading()
{
	return data[4];
}

// return reference to temperature heading data element
inline float& GeneticSimulation::SensoryData::temperature_heading()
{
	return data[5];
}

// return reference to memory data element
inline float& GeneticSimulation::SensoryData::memory()
{
	return data[6];
}