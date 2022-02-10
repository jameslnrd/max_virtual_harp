/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#include "c74_min.h"
#include <algorithm>


#include "mi_virtual_harp.h"
#include "Bridge.h"
#include "Dampers.h"
#include "BowInput.h"
#include "Obstacle.h"




using namespace c74::min;
using namespace c74::min::ui;

namespace max = c74::max;



class mi_virtual_harp : public object<mi_virtual_harp>, public ui_operator<340, 400>, public mc_operator<> {
public:
	MIN_DESCRIPTION{ "Virtual Physical Model of Harp Instrument." };
	MIN_TAGS{ "ui, audio/physics" };
	MIN_AUTHOR{ "James Leonard" };
	MIN_RELATED{ "" };

	/*
	// HACK to create multichannel output for all strings !

	//adding callback on class registration
	message<> maxclass_setup{ this, "maxclass_setup",
							 MIN_FUNCTION{
		auto c = max::class_findbyname(max::gensym("box"), max::gensym("mi_virtual_harp"));
		max::class_addmethod(c,(max::method)setOutChans,"multichanneloutputs",max::A_LONG, 0);
		 return {};
	} };

	//callback will be called on a recompilation of dsp graph
	static long setOutChans(void* obj, long outletindex) {
		if (outletindex == 0)
			return 47;
		else
			return 1;
	}
	*/
	

	// Model inputs & outputs
	inlet<> input_1{ this, "(multichannelsignal) MC input", "multichannelsignal" };
	inlet<> input_2{ this, "(signal) modulation input", "signal" };
	inlet<> input_3{ this, "(signal) contact position" };

	outlet<> output_1{ this, "(signal) output_lowL", "signal" };
	outlet<> output_2{ this, "(signal) output_lowR", "signal" };
	outlet<> output_3{ this, "(signal) output_lowMidL", "signal" };
	outlet<> output_4{ this, "(signal) output_lowMidR", "signal" };
	outlet<> output_5{ this, "(signal) output_MidL", "signal" };
	outlet<> output_6{ this, "(signal) output_MidR", "signal" };
	outlet<> output_7{ this, "(signal) output_HighL", "signal" };
	outlet<> output_8{ this, "(signal) output_HighR", "signal" };

	outlet<> output_9{ this, "(anything) message output" };


	// Visual elements (colors, positions, etc.)

	attribute<numbers> m_range{ this, "range", {{0.0, 1.0}} };
	attribute<numbers> m_offset{ this, "offset", {{60.0, 30.0}} };
	attribute<symbol>  m_label{ this, "label", "Harp Model" };
	attribute<numbers> m_posOffset{ this, "String Position Offset", {{10.0, 3.0}} };

	attribute<numbers> m_bridgeHigh{ this, "Top Bridge Position", {{20, 20.0}} };
	attribute<numbers> m_bridgeLow{ this, "Bottom Bridge Position", {{300.0, 400.0}} };

	attribute<numbers> m_obsPos{ this, "Obstacle Position", {{200.0, 200.0}} };


	attribute<number> m_outGain{ this, "Output Gain", 0.001 };

	attribute<number> m_refreshRate{ this, "Refresh Rate", 70 };


	attribute<symbol> m_fontname{ this, "fontname", "lato-light" };
	attribute<number> m_fontsize{ this, "fontsize", 14.0 };

	attribute<color> m_bgcolor{ this, "bgcolor", color::predefined::white, title {"Background Color"} };
	attribute<color> m_bordercolor{ this, "bordercolor", c74::min::ui::color::predefined::gray, title {"Border Color"} };

	attribute<color> m_strcolor{ this, "strcolor", color {{0.2, 0.2, 1, 1.}}, title {"String Color"} };

	attribute<color> m_bscolor{ this, "bscolor", color {{0.8, 0.4, 0.4, 1.}}, title {"Bridge Spring Color"} };
	attribute<color> m_bcolor{ this, "bcolor", color {{1., 0.2, 0.2, 1.}}, title {"Bridge Color"} };

	attribute<number> m_stringSpacing{ this, "spacing", 10., title{"String Spacing"} };
	attribute<number> m_stringStrech{ this, "stretch", 1., title{"String Strech (vertical)"} };
	attribute<number> m_stringZoom{ this, "zoom", 1., title{"String motion zoom"} };

	attribute<number> m_stringMassSize{ this, "sms", 2., title{"String mass size"} };
	attribute<number> m_bridgeMassSize{ this, "bs", 8., title{"Bridge mass size"} };

	attribute<color> m_contactcolor{ this, "dcolor", color {{0.2, 1, 0.2, 1.}}, title {"Contact Color"} };
	attribute<bool> m_contactActive{ this, "contactActive", 1, title{"Contacts Active"} };

	attribute<bool> m_bowActive{ this, "bowActive", 1, title{"Bow Active"} };
	attribute<bool> m_twangActive{ this, "twangActive", 1, title{"String Twang"} };

	attribute<bool> m_hardDamper{ this, "hardDamper", 1, title{"Hard Damper"} };



private:
	std::array<int, NB_STRINGS> excitePoint;
	std::array<float, NB_BOWED> bowData;

	std::array<std::array<int, 5>, NB_STRINGS> excSpread;
	std::array<std::array<int, NB_LISTEN>, NB_STRINGS> listenPoints;

	//std::array<std::vector<int>, NB_STRINGS> motionPoints;


	void print_vectors() {
		float* d;

		cout << "--- X vector: [";
		for (int i = 0; i < NB_MASSES; i++) {
			if (i > 0)
				cout << ", ";
			cout << x_ptr[i];
		}
		cout << "]" << endl;

		cout << "--- X shift: [";
		for (int i = 0; i < NB_MASSES; i++) {
			if (i > 0)
				cout << ", ";
			cout << d[i];
		}
		cout << "]" << endl;

		d = (float*)& F1[0];
		cout << "--- F1 : [";
		for (int i = 0; i < NB_MASSES; i++) {
			if (i > 0)
				cout << ", ";
			cout << d[i];
		}
		cout << "]" << endl;

		d = (float*)& F[0];
		cout << "--- F total : [";
		for (int i = 0; i < NB_MASSES; i++) {
			if (i > 0)
				cout << ", ";
			cout << d[i];
		}
		cout << "]" << endl;


		cout << "---" << endl;
	}



	// This doesn't need to change at audio rate !
	void move_string_excitation_pos(float pos, int str_index) {
		excitation[str_index] = pos;
		excitePoint[str_index] = string_starts[str_index] + ((string_size[str_index] - 1) * excitation[str_index]);
		int start = string_starts[str_index];
		int end = string_ends[str_index];

		excSpread[str_index][0] = clamp(excitePoint[str_index] - 2, start, end);
		excSpread[str_index][1] = clamp(excitePoint[str_index] - 1, start, end);
		excSpread[str_index][2] = clamp(excitePoint[str_index], start, end);
		excSpread[str_index][3] = clamp(excitePoint[str_index] + 1, start, end);
		excSpread[str_index][4] = clamp(excitePoint[str_index] + 2, start, end);
	}

	// Set excitation position for all of the strings
	void set_global_excitation_pos(float pos) {
		for (int i = 0; i < NB_STRINGS; i++)
			move_string_excitation_pos(pos, i);
	}

	// Precalculate listening points on the strings
	void set_listen_points() {
		for (auto i = 0; i < NB_STRINGS; i++) {
			listenPoints[i][0] = round(string_starts[i] + int((string_ends[i] - string_starts[i] + 1) * 0.5));
			listenPoints[i][1] = round(string_starts[i] + int((string_ends[i] - string_starts[i] + 1) * 0.666));
			listenPoints[i][2] = round(string_starts[i] + int((string_ends[i] - string_starts[i] + 1) * 0.2));
			listenPoints[i][3] = round(string_starts[i] + int((string_ends[i] - string_starts[i] + 1) * 0.86));
		}
	}
	/*
	// Precalculate motion points on the strings (for visualisation)
	void set_motion_points() {
		cout << motionPoints << endl;
		
		for (auto i = 0; i < NB_STRINGS; i++) {
			if (nbMasses[i] > 300) {
				for (auto j = 0; j < 50; j++)
					motionPoints[i].push_back(int(string_starts[i] + (string_ends[i] - string_starts[i])*j/49));
			}
			else if (nbMasses[i] > 100) {
				for (auto j = 0; j < 30; j++)
					motionPoints[i].push_back(int(string_starts[i] + (string_ends[i] - string_starts[i]) * j / 29));
			}
			else if (nbMasses[i] > 30) {
				for (auto j = 0; j < 20; j++)
					motionPoints[i].push_back(int(string_starts[i] + (string_ends[i] - string_starts[i]) * j / 19));
			}
			else if (nbMasses[i] > 10) {
				for (auto j = 0; j < 10; j++)
					motionPoints[i].push_back(int(string_starts[i] + (string_ends[i] - string_starts[i]) * j / 9));
			}
			else {
				for (auto j = 0; j < (string_ends[i] - string_starts[i]); j++)
					motionPoints[i].push_back(string_starts[i] + j);
			}
				
		}
		
	}
	*/

public:

	mi_virtual_harp(const atoms& args = {}) : ui_operator::ui_operator{ this, args } {


		// pre-compute stiffness tables for the model (optimise string stiffness and length for computation purposes)
		for (auto i = 0; i < NB_STRINGS; i++) {
			if (i < 5) {
				stiffness[i] = stiffness_min[i];
				nbMasses[i] = nbMasses_min[i];
				damping[i] = damping_7k[i];
			}
			else if (i < 25){
				stiffness[i] = stiffness_7k[i];
				nbMasses[i] = nbMass_7k[i];
				damping[i] = damping_7k[i];
			}
			else if (i < 40) {
				stiffness[i] = stiffness_9k[i];
				nbMasses[i] = nbMass_9k[i];
				damping[i] = damping_9k[i];
			}
			else if (i < NB_STRINGS) {
				stiffness[i] = stiffness_11k[i];
				nbMasses[i] = nbMass_11k[i];
				damping[i] = damping_11k[i];
			}
		}



		// initialise the vectorised string structures
		init_string_physics();

		//#if DEBUG
		cout << "nb active masses: " << nb_active_masses << endl;
		cout << "corresp vector size: " << active_vec_size << endl;
		/*
		for (int i = 0; i < nb_active_masses; i++) {
			cout << "K val for " << i << ": " << k_ptr[i] << endl;
		}
		*/
		//#endif

		// initialise other structures (needs to be after string, as they refer to it)
		init_bridge_physics();
		init_dampers_physics();

		init_obs_physics();

		// initialise bow position input modules
		init_bow_elements();

		// Pre-compute the twang stiffness modifications and enveloppe stuff
		for (int i = 0; i < NB_STRINGS; i++) {
			twangK[i] = (pow(pow(2., 10. / 1200.), 2.) - 1) * stiffness.at(i);
			twangEnv[i] = 0;
			twangEnvDecay[i] = 0.9995;
			curStiff[i] = stiffness.at(i);
		}

		// Configure the initial excitation points on the string
		set_global_excitation_pos(0.3);

		// Configure the listening points along each string
		set_listen_points();

		//set_motion_points();


		m_timer.delay(m_refreshRate);

	}



	void operator()(audio_bundle input, audio_bundle output) {
		
		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

		//auto out2 = output.samples(NB_STRINGS);    // first channel of stereo pair
		//auto out3 = output.samples(NB_STRINGS+1);    // second channel of stereo pair


		for (auto j = 0; j < input.frame_count(); j++) {

			//out2[j] = 0;
			//out3[j] = 0;

			auto mod = input.samples(input.channel_count() - 1 - NB_BOWED);
			//auto mod2 = input.samples(input.channel_count() - 1);

			auto val = clamp(mod[j], 0., 1.);
			//auto pos = mod2[j];


			// calculate bow inputs (using all same bow positions for now...)
			for (int i = 0; i < NB_BOWED; i++) {
				auto pos = input.samples(input.channel_count() - NB_BOWED + i);
				bowData[i] = (float)pos[j];
				//cout << "bow data : " << i << " " << pos << " " << bowData[i] << endl;
			}
			update_bow_inputs(bowData);

			// Get first multichannel input (force signals for the strings)
			for (auto channel = 0; channel < input.channel_count() - (1 + NB_BOWED); ++channel) {
				auto in{ input.samples(channel)[j] };
				if (channel < NB_STRINGS) {
					f_ptr[excSpread[channel][0]] += 0.8 * in;
					f_ptr[excSpread[channel][1]] += 1 * in;
					f_ptr[excSpread[channel][2]] += in;
					f_ptr[excSpread[channel][3]] += 1 * in;
					f_ptr[excSpread[channel][4]] += 0.8 * in;
				}
			}

			// Link Phase
			calculate_bridge_springs();
			decrease_damper_friction();
			calculate_dampers();

			if(m_contactActive)
				calculate_obs_contact();

			

			if(m_bowActive)
				calculate_bow_friction();

			// Update any dynamical modifications to string parameters
			// Could be smarter but this is actually very low impact on CPU
			
			if (m_twangActive) {
				for (int i = 0; i < NB_STRINGS; i++) {
					float k = curStiff[i];
					if (twangEnv[i] > 0.00001) {
						k = k + twangK[i] * twangEnv[i];
						twangEnv[i] *= twangEnvDecay[i];
						change_stiffness(i, k);
						bSpringK[i] = k;
					}
				}
			}
			
			
			

			// In-between : the strings (integrated objects)
			calculate_string_physics();

			// Mass Phase
			calculate_bridge_masses();

			// Listen to the string at selected points (the output is in mono for now)
			for (int i = 0; i < 8; i++)
				output.samples(i)[j] = 0;
			
			for (int i = 0; i < NB_STRINGS; i++) {
				auto outIdx = 2*std::min(i / 11, 3);
				output.samples(outIdx)[j] += m_outGain * (x_ptr[listenPoints[i][0]] + x_ptr[listenPoints[i][2]]);
				output.samples(outIdx+1)[j] += m_outGain * (x_ptr[listenPoints[i][1]] + x_ptr[listenPoints[i][3]]);
				// Multi-channel output
				/*output.samples(i)[j] =	0.2 * x_ptr[listenPoints[i][0]] +
										0.2 * x_ptr[listenPoints[i][1]] +
										0.2 * x_ptr[listenPoints[i][2]] +
										0.2 * x_ptr[listenPoints[i][3]];*/
				// stereo output
				/*out2[j] += 0.2 * x_ptr[listenPoints[i][0]] +
					0.2 * x_ptr[listenPoints[i][1]];
				out3[j] += 0.2 * x_ptr[listenPoints[i][2]] +
					0.2 * x_ptr[listenPoints[i][3]];
				*/
			}
		}

		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
		
	}


	// respond to the bang message to do something
	message<threadsafe::yes> anything{
		this, "anything", "Set the Parameters", MIN_FUNCTION {

			auto f = 0;
			auto type = args[0];

			if (type == "BridgeParams") {
				cout << "Setting Bridge Properties" << endl;

				if (args.size() > 3) {
					biM = 1. / (float)args[1];
					bK = (float)args[2];
					bZ = (float)args[3];

					update_bridge_params();
				}
				else
					cout << "Not enough arguments for function" << endl;
			}
			/*
			else if (type == "Pedals") {
				// cout << "Setting Pedal Properties" << endl;
				if (args.size() > 7) {
					//lock lock {m_mutex};

					for (auto i = 2; i < m_strings.size(); i++) {
						int  pedal = args[(i - 2) % NB_PEDALS + 1];
						double newK  = 0;
						//if (i < crossover)
						//	newK  = pow(pow(2., pedal / 12.), 2.) * stiffnessSmaller.at(i);
						//else
						newK = pow(pow(2., pedal / 12.), 2.) * stiffness.at(i);

						// cout << " newK " << newK << endl;
						m_strings[i].setK(newK);
						m_bridgeSprings[2 * i].setK(newK);
						m_bridgeSprings[2 * i + 1].setK(newK);
					}
					//lock.unlock();
				}
				else
					cout << "Not enough arguments to set the pedal state" << endl;
			}
			*/
			else if (type == "TuneString") {
				// cout << "Tuning Individual String" << endl;
				if (args.size() > 3) {
					int idx = args[1];
					if ((idx >= 0) && (idx < NB_STRINGS)) {
						float string_tune = (float)args[2];

						// Remove the damper for this string as it is about to be played
						dampZ[idx] = 0;

						// veclocity dependant component: how high the pitch is raised at the onset
						twangEnv[idx] = args[3];

						float newK = 0;
						newK = pow(pow(2., string_tune / 12.), 2.) * stiffness.at(idx);
						change_stiffness(idx, newK);
						bSpringK[idx] = newK;
						curStiff[idx] = newK;
					}
					else
						cout << "Trying to change pitch of out of range string" << endl;
				}
				else
					cout << "Not enough arguments to tune the string" << endl;
			}

			else if (type == "DumpStiffness") {
				if (args.size() > 1) {
					int idx = (int)args[1];
					if ((idx == 0) || (idx == NB_STRINGS - 1)) {
						cout << "---dumping string params---" << endl;
						for (int j = string_starts[idx]; j < string_ends[idx]; j++)
							cout << "K val for idx " << j << " [vector: " << (int)j / VECTORLEN << "] is : " << k_ptr[j] << endl;
					}
					else {
						cout << "---dumping string params---" << endl;
						for (int j = string_starts[idx] - 2; j < string_ends[idx] + 2; j++)
							cout << "K val for idx " << j << " [vector: " << (int)j / VECTORLEN << "] is : " << k_ptr[j] << endl;
					}

				}
			}

			else if (type == "AdjustTwang") {
				if (args.size() > 2) {

					float pitch = (float)args[1];
					float duration = std::min((float)1.,(float)args[2]);

					for (int i = 0; i < NB_STRINGS; i++) {
						twangK[i] = (pow(pow(2., pitch / 1200.), 2.) - 1) * stiffness.at(i);
						twangEnv[i] = 0;
						twangEnvDecay[NB_STRINGS - 1 - i] = duration * pow(0.9999,NB_STRINGS - 1 - i);
						curStiff[i] = stiffness.at(i);
					}

				}
			}

			else if (type == "ExPos") {
				if (args.size() > 2) {

					int    string_index = args[1];
					float pluckPos = (float)clamp((double)args[2], 0., 1.);

					if ((string_index >= 0) && (string_index < NB_STRINGS))
						move_string_excitation_pos(pluckPos, string_index);
					else
						cout << "Trying to change excitation point of out of range string" << endl;
				}
				else
					cout << "Not enough arguments to change the excitation point on the string" << endl;
			}

			else if (type == "PluckPosition") {
				if (args.size() > 1) {
					float pluckPos = (float)clamp((double)args[1], 0., 1.);
					set_global_excitation_pos(pluckPos);
				}
				else
					cout << "Not enough arguments to change the excitation point on all strings" << endl;
			}

			else if (type == "MoveObstacles") {
				if (args.size() > 1) {
					float obs = (float)clamp((double)args[1], 0., 1.);
					move_obstacles(obs);
				}
				else
					cout << "Not enough arguments to change the obstacle position" << endl;
			}

			else if (type == "ObstacleParams") {
			if (args.size() > 3) {
				float Q = (float)clamp((double)args[1], 0., 100.);
				float Qlim = (float)clamp((double)args[2], 0., 0.2);
				float Z = (float)clamp((double)args[3], 0., 0.01);
				update_obs_params(Q, Qlim, Z);
			}
			else
				cout << "Not enough arguments to change the obstacle parameters" << endl;
			}


			else if (type == "Damper") {
				float d = damper_soft;
				// cout << "Damping String(s)" << endl;
				if (args.size() > 2) {

					if (m_hardDamper)
						d = damper_hard;

					int idx = args[1];
					if ((idx >= -1) && (idx <= NB_STRINGS)) {
						float damp = args[2];
						if (damp >= 0) {
							float newZ = d * damp;
							if (idx == -1) {
								for (int i = 0; i < NB_STRINGS; i++)
									dampZ[i] = newZ;
							}
							else
								dampZ[idx] = newZ;
						}
					}
					else
						cout << "Trying to change damper on out of range string" << endl;
				}
				else
					cout << "Not enough arguments to dampen the string" << endl;
			}


			else if (type == "BowNoteOn") {
				if (args.size() > 1) {
					int  str = (int)args[1];
					//cout << "bow note on: " << str << endl;

					auto inList = false;
					for (auto i = 0; i < NB_BOWED; i++) {
						if ((bow_string[i] == str) && (bow_state[i] == 1)) {
							inList = true;
						}
					}
					if (inList == false) {
						for (auto i = 0; i < NB_BOWED; i++) {
							if (bow_state[i] == 0) {
								bow_string[i] = str;
								bow_state[i] = 1;
								break;
							}
						}
					}
					/*for (auto i = 0; i < NB_BOWED; i++) {
						cout << "bow state " << i << " : " << bow_state[i] << "->" << bow_string[i] << endl;
					}*/

				}
			}

			else if (type == "BowNoteOff") {
				if (args.size() > 1) {
					int  str = (int)args[1];
					//cout << "Bow Note Off : " << str << endl;

					auto inList = false;
					for (auto i = 0; i < NB_BOWED; i++) {
						if ((bow_string[i] == str)) {
							bow_state[i] = 0; 
						}
						//cout << "bow state" << i << ":" << bow_state[i] << "->" << bow_string[i] << endl;
					}
				}
            
			}
            

			else if (type == "BowFric") {
				if (args.size() > 1) {
					float nlZ = (float)args[1];
					for (auto i = 0; i < NB_BOWED; i++)
						frictionZ[i] = nlZ;
				}
				else
					cout << "Not enough arguments to change the bowing friction" << endl;
			}

			else if (type == "BowScale") {
				if (args.size() > 1) {
					float scale = (float)args[1];
					for (auto i = 0; i < NB_BOWED; i++)
						frictionScale[i] = scale;
				}
				else
					cout << "Not enough arguments to change bowing scale" << endl;
			}

			else if (type == "BowPoint") {
			if (args.size() > 1) {
				float point = (float)args[1];
				for (auto i = 0; i < NB_BOWED; i++)
					friction_points[i] = clamp((float)point, (float)0., (float)1.); ;
			}
			else
				cout << "Not enough arguments to change bowing point" << endl;
			}

			return {};
		}
	};


	message<> paint{
		this, "paint", MIN_FUNCTION {
			target t {args};
			rect<fill> {// background
				t, color{ m_bgcolor }};
			rect<> {// frame
				t, color{ m_bordercolor }, line_width{ 5.0 }};

			float vertical = 0;

			for (int i = 0; i < NB_STRINGS; i++) {

				vertical = 0;
				for (int j = string_starts[i]; j <= string_ends[i]; j++) {
					//auto c = offset + j;
					ellipse<fill> {t, color{ m_strcolor },
						position{ m_offset[0] + m_stringSpacing * i + x_ptr[j] * m_stringZoom - 0.5 * m_stringMassSize, m_offset[1] + vertical - 0.5 * m_stringMassSize },
						size{ m_stringMassSize, m_stringMassSize }};
					vertical += (float)m_stringStrech;
				}
				line<> {t, color{ m_bscolor },
					origin{ m_offset[0] + m_stringSpacing * i + x_ptr[string_ends[i]] * m_stringZoom, m_offset[1] + vertical },
					destination{ m_offset[0] + m_bridgeLow[0] + bridges_x[1] * m_stringZoom, m_offset[1] + m_bridgeLow[1] }, line_width{ 1. }};
				line<> {t, color{ m_bscolor },
					origin{ m_offset[0] + m_stringSpacing * i + x_ptr[string_starts[i]] * m_stringZoom, m_offset[1] },
					destination{ m_offset[0] + m_bridgeHigh[0] + bridges_x[0] * m_stringZoom, m_offset[1] + m_bridgeHigh[1] }, line_width{ 1. }};

				
				if (m_contactActive){
					line<> {t, color{ m_contactcolor },
					origin{ m_offset[0] + m_stringSpacing * i, m_offset[1] + obsLocation * vertical },
					destination{ m_offset[0] + m_obsPos[0] - 0.5 * m_bridgeMassSize + obs_x * m_stringZoom, m_offset[1] + m_obsPos[1] - 0.5 * m_bridgeMassSize }, line_width{ 1. }};
				}
				
				if (m_hardDamper) {
					ellipse<> {t, color{ {1., 0.5, 0.5, dampZ[i] * 30.} },
						position{ m_offset[0] + m_stringSpacing * i - 3, m_offset[1] + string_size[i] * 0.3 - 3 }, size{ 6., 6.0 }};
				}
				else {
					ellipse<> {t, color{ {0.5, 0.5, 1, dampZ[i] * 900.} },
						position{ m_offset[0] + m_stringSpacing * i - 3, m_offset[1] + string_size[i] * 0.3 - 3 }, size{ 6., 6.0 }};
				}


			}

			ellipse<fill> {t, color{ m_bcolor },
				position{ m_offset[0] + m_bridgeHigh[0] - 0.5 * m_bridgeMassSize + bridges_x[0] * m_stringZoom, m_offset[1] + m_bridgeHigh[1] - 0.5 * m_bridgeMassSize },
				size{ m_bridgeMassSize, m_bridgeMassSize }};
			ellipse<fill> {t, color{ m_bcolor },
				position{ m_offset[0] + m_bridgeLow[0] - 0.5 * m_bridgeMassSize + bridges_x[1] * m_stringZoom, m_offset[1] + m_bridgeLow[1] - 0.5 * m_bridgeMassSize },
				size{ m_bridgeMassSize, m_bridgeMassSize }};

			/*
			if(m_contactActive){
			ellipse<fill> {t, color{ m_contactcolor },
				position{ m_offset[0] + 350 - 0.5 * m_bridgeMassSize + obs_x * m_stringZoom, m_offset[1] + 200 - 0.5 * m_bridgeMassSize },
				size{ m_bridgeMassSize, m_bridgeMassSize }};
			}
			*/

			return {};
		}
	};

	void stringMessages() {

		atoms m_data;
		symbol tes2;
		//m_data.reserve(199);
		//m_data.insert(m_data.end(), 1);
		//m_data.insert(m_data.end(), 2);
		//m_data.insert(m_data.end(), 3);


		for (auto i = 0; i < NB_STRINGS; i++) {
			m_data.clear();
			std::string str = "/string";
			//str.append(std::to_string(i));
			symbol sn = str;
			m_data.insert(m_data.end(), sn);
			m_data.insert(m_data.end(), i);
			for (int j = string_starts[i]; j <= string_ends[i]; j = j + 10) {
				m_data.insert(m_data.end(), x_ptr[j]);
			}
			output_9.send(m_data);
		}

		output_9.send(m_data);

	}

	timer<timer_options::defer_delivery> m_timer{
		this, MIN_FUNCTION {
			redraw();
			m_timer.delay(m_refreshRate);
			//output_9.send("test");
			stringMessages();
			return {};
		}
	};


private:

	int                             m_state_idx;
}
;

MIN_EXTERNAL(mi_virtual_harp);
