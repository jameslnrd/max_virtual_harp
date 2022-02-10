#include "Dampers.h"

#include "Globals.h"

float dampZ[NB_STRINGS];

int idx1[NB_STRINGS];
int idx2[NB_STRINGS];
int idx3[NB_STRINGS];
int idx4[NB_STRINGS];

void init_dampers_physics() {
	for(int i = 0; i < NB_STRINGS; i++){
		dampZ[i] = 0.1;
		idx1[i] = string_starts[i] + (int)((string_size[i] - 1) * 0.05);
		idx2[i] = string_starts[i] + (int)((string_size[i] - 1) * 0.36);
		idx3[i] = string_starts[i] + (int)((string_size[i] - 1) * 0.50);
		idx4[i] = string_starts[i] + (int)((string_size[i] - 1) * 0.75);
	}
}

void decrease_damper_friction() {
	for (auto i = 0; i < NB_STRINGS; i++)
		dampZ[i] *= 0.99995;
}

void calculate_dampers() {
	float f = 0;
	
	// Unilateral damping interactions on string masses
	for (auto i = 0; i < NB_STRINGS; i++) {
		f = dampZ[i] * ((x_ptr[idx1[i]] - xp_ptr[idx1[i]]));
		f_ptr[idx1[i]] -= f;
		
		f = dampZ[i] * ((x_ptr[idx2[i]] - xp_ptr[idx2[i]]));
		f_ptr[idx2[i]] -= f;

		f = dampZ[i] * ((x_ptr[idx3[i]] - xp_ptr[idx3[i]]));
		f_ptr[idx3[i]] -= f;

		f = dampZ[i] * ((x_ptr[idx4[i]] - xp_ptr[idx4[i]]));
		f_ptr[idx4[i]] -= f;
	}
}