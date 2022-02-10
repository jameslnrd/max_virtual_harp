
#include "Globals.h"
#include "Obstacle.h"

float obs_x;

float contactK;
float contactLimK;
float contactZ;

float B_param;

int obsIndexes[NB_STRINGS];
float obsLocation = 0;

void update_obs_params(float Q, float Qlim, float Z) {
	contactK = Q;
	contactLimK = Qlim;
	contactZ = Z;
	B_param = sqrt(contactLimK / contactK);
}


void init_obs_physics() {
	update_obs_params(0.01, 0.2, 0.0001);
	move_obstacles(0.3);
}

void move_obstacles(float a) {
	if (a < 0)
		a = 0;
	if (a > 1)
		a = 1;
	for (auto i = 0; i < NB_STRINGS; i++)
		obsIndexes[i] = (int)(string_starts[i] + a * (string_ends[i] - string_starts[i]));
	obsLocation = a;
}


void calculate_obs_contact() {
	float f = 0;
	float var_dx;
	float absdx;
	float b;

	/*
	int idx1, idx2;
	float alpha = 0;
	float loc;
	float pos, posR;
	*/

	int idx;
	for (auto i = 0; i < NB_STRINGS; i++) {
		/*
		loc = string_starts[i] + 0.1 * (string_ends[i] - string_starts[i]);

		idx1 = floor(loc);
		idx2 = idx1+1;
		alpha = loc - idx1;

		if (idx2 < string_ends[i])
			idx2 = string_ends[i];

		pos = (1. - alpha) * x_ptr[idx1] + alpha * x_ptr[idx2];
		posR = (1. - alpha) * xp_ptr[idx1] + alpha * xp_ptr[idx2];
		*/

		idx = obsIndexes[i];
		var_dx = x_ptr[idx] - obs_x;
		absdx = abs(var_dx);

		if (var_dx > 0) {
			// Either calculate cubic and linear force, or bounded linear force
			if (absdx <= B_param)
				f = var_dx * (contactK * var_dx * var_dx);
			else
				f = contactLimK * var_dx;
			f += contactZ * ((x_ptr[idx] - xp_ptr[idx]));
			f_ptr[idx] -= f;
		}
	}
}