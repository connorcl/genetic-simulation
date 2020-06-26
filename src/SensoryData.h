#pragma once

#include <vector>

namespace GeneticSimulation
{
	// Stores the sensory data of an organism and makes 
	// this available in original and scaled [-1, 1] form
	class SensoryData
	{
	public:

		// constructor
		SensoryData();

		// returns hunger value
		float get_hunger();

		// returns thirst value
		float get_thirst();

		// returns temperature damage value
		float get_temperature_damage();

		// gets heading to nearest food
		float get_food_heading();

		// gets heading to nearest water
		float get_water_heading();

		// gets direction to best temperature
		float get_temperature_heading();

		// gets memory item
		float get_memory();

		// returns a reference to the vector of scaled sensory values
		const std::vector<float>& get_data();

		// sets scaled hunger value based on nutrition
		void set_hunger(unsigned int nutrition);

		// sets scaled thirst value based on hydration
		void set_thirst(unsigned int hydration);

		// set scaled temperature damage value based on integrity
		void set_temperature_damage(unsigned int integrity);

		// set scaled food heading
		void set_food_heading(float heading);

		// set scaled water heading
		void set_water_heading(float heading);

		// set scaled temperature heading
		void set_temperature_heading(float heading);

		// set memory item
		void set_memory(float memory_item);

	private:

		// return reference to correct data element
		inline float& hunger();
		inline float& thirst();
		inline float& temperature_damage();
		inline float& food_heading();
		inline float& water_heading();
		inline float& temperature_heading();
		inline float& memory();

		// vector of scaled data values
		std::vector<float> data;
	};
}