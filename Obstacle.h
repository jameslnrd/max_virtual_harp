#pragma once

extern float obs_x;

extern float contactK;
extern float contactLimK;

extern int obsIndexes[];
extern float obsLocation;

void update_obs_params(float Q, float Qlim, float Z);

void init_obs_physics();

void move_obstacles(float a);

void calculate_obs_contact();