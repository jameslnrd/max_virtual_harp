#pragma once

//#include "mi_virtual_harp.h"
#include "Globals.h"

#include <xmmintrin.h>
#include <immintrin.h>

extern __m256 F1[];
extern __m256 dX[];
extern __m256 dV[];

extern float* xs_ptr;
extern float* vs_ptr;

extern __m256 str_invM[];
extern __m256 str_K[];
extern __m256 str_Z[];

extern __m256 inv_mass;
extern __m256 shifted;
extern __m256 tmp;
extern __m256 A;
extern __m256 B;


extern int nb_active_masses;
extern int active_vec_size;


void init_string_physics();

void calculate_string_physics();

void change_stiffness(int idx, float K);