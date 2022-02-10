#pragma once

extern float bridges_x[];
extern float bridges_xp[];
extern float bridges_f[];

extern float biM;
extern float bK;
extern float bZ;

extern float bSpringK[];
extern float bSpringZ[];

void update_bridge_params();

void init_bridge_physics();

void calculate_bridge_masses();

void calculate_bridge_springs();