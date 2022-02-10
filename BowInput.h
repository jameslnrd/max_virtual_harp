#pragma once
#include <array>
#include "Globals.h"

extern float bow_x[];
extern float bow_xp[];
extern float bow_f[];

extern float frictionZ[];
extern float frictionScale[];

extern float friction_points[];

// New stuff, related to bowing system

extern std::array<int, NB_BOWED>    bow_string;
extern std::array<int, NB_BOWED>   bow_state;

extern float                         bow_pos;
extern float                         bow_pres;



void init_bow_elements();

void update_bow_inputs(std::array<float, NB_BOWED> newPositions);

void calculate_bow_friction();