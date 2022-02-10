#include "Globals.h"


float bow_x[NB_BOWED];
float bow_xp[NB_BOWED];
float bow_f[NB_BOWED];

float frictionZ[NB_BOWED];
float frictionScale[NB_BOWED];

float friction_points[NB_BOWED];
int friction_indexes[NB_BOWED];

std::array<int, NB_BOWED>   bow_string;
std::array<int, NB_BOWED>   bow_state;

float                         bow_pos;
float                         bow_pres;



void init_bow_elements() {
	
	for (auto i = 0; i < NB_BOWED; i++) {

		// initialise the bow position input elements
		bow_x[i] = 0;
		bow_xp[i] = 0;
		bow_f[i] = 0;

		// initialise the friction interaction elements
		frictionZ[i] = 0.5;
		frictionScale[i] = 0.02;
		friction_points[i] = 0.18;

	}

}

void update_bow_inputs(std::array<float, NB_BOWED> newPositions) {
	for (auto i = 0; i < NB_BOWED; i++) {
		float tmp = bow_x[i];
		bow_x[i] = newPositions[i]; 
		bow_xp[i] = tmp;
		bow_f[i] = 0;
	}
}

void calculate_bow_friction() {

	for (auto i = 0; i < NB_BOWED ; i++) {

		if(bow_state[i]){

			// some fake indexes just to apply the bow onto something
			int idx = string_starts[bow_string[i]] + (string_ends[bow_string[i]]-string_starts[bow_string[i]])* friction_points[i];

			// calculate relatve velocity of bow and of string masses
			float vel = ((x_ptr[idx] - xp_ptr[idx]) - (bow_x[i] - bow_xp[i]));

			// friction à la Bilbao's book
			float f = 0.63 * frictionZ[i] * vel * exp(-4. * pow(vel / frictionScale[i], 2) + 0.5);


			//float f = frictionZ[i] * sqrt(2 * frictionScale[i]) * vel * exp(-frictionScale[i] * vel * vel + 1 / 2);

			f_ptr[idx] -= f;
			bow_f[i] += f;
		
		}

	}
}
