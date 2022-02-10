#include "StringPhysics.h"
#include <iostream>

__m256 F1[AVX_SV];
__m256 dX[AVX_SV];
__m256 dV[AVX_SV];
__m256 tmpVec[AVX_SV];

float* xs_ptr;
float* vs_ptr;

__m256 inv_mass;
__m256 shifted;
__m256 tmp;
__m256 A;
__m256 B;

__m256 Kv;

int nb_active_masses;
int active_vec_size;

float Zo = 0.00002;


void init_string_physics() {


	// Strings init phase
	for (auto i = 0; i < AVX_SV; i++)
	{
		X[i] = _mm256_set1_ps(0.);
		XP[i] = _mm256_set1_ps(0.);
		F[i] = _mm256_setzero_ps();
	}

	inv_mass = _mm256_set1_ps(1.0);

	k_ptr = (float*)& str_K[0];
	z_ptr = (float*)& str_Z[0];
	m_ptr = (float*)& str_invM[0];

	int idx = 0;
	nb_active_masses = 0;

	// Setting parameters for the strings
	for (auto i = 0; i < NB_STRINGS; i++) {
		// Fill the parameters for this string
		string_starts[i] = idx;
		string_size[i] = nbMasses.at(i);
		nb_active_masses += nbMasses.at(i);

		string_start_vec[i] = (int)floor(string_starts[i] / VECTORLEN);


		for (int j = 0; j < nbMasses.at(i) - 1 ; j++) {
			*k_ptr++ = -(float)stiffness.at(i);
			*z_ptr++ = -(float)damping.at(i);
			//*m_ptr++ = 0.95 + ((float)rand() / (float)(RAND_MAX)) * 0.1;
			idx++;
		}

		string_ends[i] = idx;
		string_end_vec[i] = (int)floor(string_ends[i] / VECTORLEN);

		// Fill with zeroes in between strings
		*k_ptr++ = 0;
		*z_ptr++ = 0;
		//*m_ptr++ = 0.95 + ((float)rand() / (float)(RAND_MAX)) * 0.1;
		idx++;
	}


	active_vec_size = (int)ceil(nb_active_masses / (float)VECTORLEN);

	std::cout << "Active vector size: " << active_vec_size << std::endl;

	std::cout << "First and last masses" << std::endl;
	for (auto i = 0; i < NB_STRINGS; i++) {
		std::cout << "String " << i << " : " << string_starts[i] << " " << string_ends[i] << std::endl;
		//cout << "Coupling springs " << i << " : " << bSpringK[i] << " " << bSpringK[i] << endl;
	}

	// Here, we set individual mass to one !
	A = _mm256_set1_ps(2 - Zo * 1);
	B = _mm256_set1_ps(Zo * 1 - 1);

	//A = _mm256_set1_ps(2);

	// Float pointers
	x_ptr = (float*)& X[0];
	xp_ptr = (float*)& XP[0];
	f_ptr = (float*)& F[0];
	v_ptr = (float*)& V[0];

	k_ptr = (float*)& str_K[0];
	z_ptr = (float*)& str_Z[0];


	x_ptr[2] = 0.1;

}



void calculate_string_physics() {
	
	// Mass algorithm
	for (int i = 0; i < active_vec_size; i++) {
		// Store prev value of X
		tmp = X[i];
		// Xn = 2 * X - Xp + F/M
		X[i] = _mm256_fmadd_ps(F[i], inv_mass, _mm256_fmadd_ps(X[i], A, _mm256_mul_ps(XP[i], B)));
		// Fill V
		V[i] = _mm256_sub_ps(X[i], tmp);
		// Xp = tmp
		XP[i] = tmp;
	}

	

	// Create shifted position & speed vectors for the {k <=> k+1} string interactions
	float leftoverX = 0, leftoverV = 0, crossoverF = 0;

	// Could probably do this better with smart permutation operations, but they're a pain to understand...
	// To shift all values left we need to go through the loop backwards and report the terms that slide into previous vectors
	for (int i = active_vec_size - 1; i >= 0; i--) {
		float* d = (float*)& X[i];
		float* e = (float*)& V[i];
		dX[i] = _mm256_sub_ps(X[i], _mm256_set_ps(leftoverX, d[7], d[6], d[5], d[4], d[3], d[2], d[1]));
		leftoverX = d[0];
		dV[i] = _mm256_sub_ps(V[i], _mm256_set_ps(leftoverV, e[7], e[6], e[5], e[4], e[3], e[2], e[1]));
		leftoverV = e[0];
	}

	for (int i = 0; i < active_vec_size; i++) {
		// F = - K deltaX - Z * deltaV
		//F1[i] = _mm256_add_ps(_mm256_mul_ps(str_K[i], dX[i]), _mm256_mul_ps(str_Z[i], dV[i]));
		F1[i] = _mm256_fmadd_ps(str_K[i], dX[i], _mm256_mul_ps(str_Z[i], dV[i]));
	}

	// Could probably do this better with smart permutation operations, but they're a pain to understand...
	// Create a shifted vector with the opposite forces and add them to the Force vector
	for (int i = 0; i < active_vec_size; i++) {
		float* d = (float*)& F1[i];
		F[i] = _mm256_add_ps(F1[i], _mm256_set_ps(-d[6], -d[5], -d[4], -d[3], -d[2], -d[1], -d[0], crossoverF));
		crossoverF = -d[7];
	}
}


void change_stiffness(int idx, float K) {

	Kv = _mm256_set1_ps(-K);
	
	// Get the first AVX vector for this string (but possibly with other string data in it)
	for (int i = string_starts[idx]; i < std::min(string_ends[idx], VECTORLEN * (string_start_vec[idx]+1)); i++)
		k_ptr[i] = -K;

	// Do what we can on the full vector section
	for (int i = string_start_vec[idx]+1; i < string_end_vec[idx]; i++)
		str_K[i] = Kv;

	// Finish the job with the last elements (but don't overwrite other data)
	for (int i = std::max(string_starts[idx], VECTORLEN * (string_end_vec[idx])); i < string_ends[idx]; i++)
		k_ptr[i] = -K;

}