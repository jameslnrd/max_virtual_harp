#pragma once

//#include "Phy.Context/NodeStateTable.h"

#include <array>
#include <immintrin.h>



#define NB_STRINGS 47
#define NB_PEDALS 7

#define NB_BOWED 10
#define NB_OSC 2

#define NB_LISTEN 4

#define NB_MASSES 6000
#define VECTORLEN 8
#define AVX_SV NB_MASSES / VECTORLEN

// Global string attributes that must be shared by the rest of the model (for link calculations, etc.)

extern __m256 XP[];
extern __m256 X[];
extern __m256 F[];
extern __m256 V[];

extern float* x_ptr;
extern float* xp_ptr;
extern float* f_ptr;
extern float* v_ptr;

extern int string_starts[];
extern int string_ends[];
extern int string_size[];

extern int string_start_vec[];
extern int string_end_vec[];

extern __m256 str_M[];
extern __m256 str_K[];
extern __m256 str_Z[];

extern float* k_ptr;
extern float* z_ptr;
extern float* m_ptr;

extern std::array<float, NB_STRINGS> excitation;

extern std::array<float, NB_STRINGS> damping_old;

extern std::array<int, NB_STRINGS> nbMasses_big;
extern std::array<int, NB_STRINGS> nbMasses_reg;
extern std::array<int, NB_STRINGS> nbMasses_min;


extern std::array<float, NB_STRINGS> stiffness_big;
extern std::array<float, NB_STRINGS> stiffness_reg;
extern std::array<float, NB_STRINGS> stiffness_min;


extern std::array<int, NB_STRINGS> nbMasses;
extern std::array<float, NB_STRINGS> stiffness;
extern std::array<float, NB_STRINGS> damping;


extern std::array<float, NB_STRINGS> twangK;
extern std::array<float, NB_STRINGS> twangEnv;
extern std::array<float, NB_STRINGS> twangEnvDecay;

extern std::array<float, NB_STRINGS> curStiff;

extern std::array<int, NB_STRINGS> nbMass_7k;
extern std::array<float, NB_STRINGS> stiffness_7k;
extern std::array<float, NB_STRINGS> damping_7k;

extern std::array<int, NB_STRINGS> nbMass_8k;
extern std::array<float, NB_STRINGS> stiffness_8k;
extern std::array<float, NB_STRINGS> damping_8k;

extern std::array<int, NB_STRINGS> nbMass_9k;
extern std::array<float, NB_STRINGS> stiffness_9k;
extern std::array<float, NB_STRINGS> damping_9k;

extern std::array<int, NB_STRINGS> nbMass_10k;
extern std::array<float, NB_STRINGS> stiffness_10k;
extern std::array<float, NB_STRINGS> damping_10k;

extern std::array<int, NB_STRINGS> nbMass_11k;
extern std::array<float, NB_STRINGS> stiffness_11k;
extern std::array<float, NB_STRINGS> damping_11k;

extern std::array<int, NB_STRINGS> nbMass_12k;
extern std::array<float, NB_STRINGS> stiffness_12k;
extern std::array<float, NB_STRINGS> damping_12k;


extern int crossover;

extern float damper_hard;
extern float damper_soft;

