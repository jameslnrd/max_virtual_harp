
#include "Globals.h"


float bridges_x[NB_OSC];
float bridges_xp[NB_OSC];
float bridges_f[NB_OSC];

float biM;
float bK;
float bZ;

float bSpringK[NB_STRINGS];
float bSpringZ[NB_STRINGS];

float Aa;
float Bb;

void update_bridge_params() {
	Aa = 2 - (bK + bZ) * biM;
	Bb = bZ * biM - 1;
}


void init_bridge_physics() {
	// Configure the bridge modules.
	for (auto i = 0; i < NB_OSC; i++) {
		bridges_x[i] = 0;
		bridges_xp[i] = 0;
		bridges_f[i] = 0;
	}

	for (auto i = 0; i < NB_STRINGS; i++) {
		bSpringK[i] = (float)stiffness.at(i);
		bSpringZ[i] = (float)damping.at(i);
	}
	biM = 1./150.;
	bK = 6;
	bZ = 10;

	update_bridge_params();
}




void calculate_bridge_masses() {
	for (auto i = 0; i < NB_OSC; i++) {
		float tmp = bridges_x[i];
		bridges_x[i] = bridges_x[i] * Aa + bridges_xp[i] * Bb + bridges_f[i] * biM;
		bridges_xp[i] = tmp;
		bridges_f[i] = 0;
	}
}

void calculate_bridge_springs() {
	float f = 0;
	float fnl = 0;
	float fnlim = 0;
	int idx;
	for (auto i = 0; i < NB_STRINGS; i++) {
		// springs attached to top of strings
		idx = string_starts[i];
		f = bSpringK[i] * (x_ptr[idx] - bridges_x[0])
			+ bSpringZ[i] * ((x_ptr[idx] - xp_ptr[idx]) - (bridges_x[0] - bridges_xp[0]));
		f_ptr[string_starts[i]] -= f;
		bridges_f[0] += f;

		// springs attached to bottom of springs
		idx = string_ends[i];
		f = bSpringK[i] * (x_ptr[idx] - bridges_x[1])
			+ bSpringZ[i] * ((x_ptr[idx] - xp_ptr[idx]) - (bridges_x[1] - bridges_xp[1]));
		f_ptr[string_ends[i]] -= f;
		bridges_f[1] += f;
	}
}