// speech_util.cpp : Defines the entry point for the console application.//

#include "stdafx.h"
#include "iostream"
#include "fstream"
#include "string"

using namespace std;

char* input_file = "input.txt";
char* normalized_file = "Normalized.txt";
char* energy_file = "energy.txt";
char* silence_file = "silence.txt";
char* trimmed_file = "trim.txt";


ifstream in, in_s, in_t;
ofstream out, out_t;

int flag = 0;
int trim_start_value = 1500;
int energy_file_count = 0;
double current_value;
long int start_marker = 0;
long int end_marker = 0;
long double total_energy = 0;
long int lcr_threshold = 5000;
long int lcr_count = 0;
long int zcr_count = 0;
int remove_header_counter = 5;
long int max_sample_value = 1;
double sum_samples = 0;
long int no_of_samples = 0;
double dc_shift_value = 0;
double normalization_ratio = 1;

//Initialize File names
void intialize_file_names(){
	//normalized_file  = strcat("normalized_", input_file);
	//energy_file = strcat("energy_", input_file);
	//normalized_file = "normalized_" + input_file;
	//energy_file = "energy_" + input_file;
}

//To Remove header
void remove_header(){
	if (flag){
		remove_header_counter = 5;
		string temp = "";
		while (getline(in_s, temp) && remove_header_counter){
			//cout << temp<<endl;
			remove_header_counter--;
		}
		flag = 0;
	}
	else{
		remove_header_counter = 5;
		string temp = "";
		while (getline(in, temp) && remove_header_counter){
			//cout << temp<<endl;
			remove_header_counter--;
		}
	}
}

//To calculate DC Shift
void calculate_dc_shift(){
	flag = 1;
	cout << "\n................Calculating DC shift..................." << endl;
	in_s.open(silence_file);
	remove_header();
	string temp = "";

	while (in_s >> temp && !remove_header_counter){
		current_value = stoi(temp);
		sum_samples += current_value;
		no_of_samples++;
	}

	dc_shift_value = sum_samples / no_of_samples;
	cout << "DC Shift value:" << dc_shift_value << endl;
	in_s.clear();
	in_s.seekg(0);
	in_s.close();
}

//To calculate normalization ratio
void calculate_normalization_ratio(){	
	remove_header();
	string temp = "";
	cout << "\n............Calculating Normalization Ratio................"<<endl;
	while (in >> temp && !remove_header_counter){
		current_value = stoi(temp);
		if (current_value > max_sample_value)
			max_sample_value = current_value;
	}
	cout << "Max Sample value:" << max_sample_value << endl;

	normalization_ratio = 5000.0 / max_sample_value;
	cout << "Normalization ratio:" << normalization_ratio << endl;

	in.clear();
	in.seekg(0);
}

//To remove DC Shift and normalize
void dc_normalize(){

	remove_header();
	string temp = "";
	cout << "\n........Removing DC shift and Normalizing File.........." << endl;
	out.open(normalized_file);

	while (in >> temp && !remove_header_counter){
		current_value = stoi(temp);
		current_value = current_value - dc_shift_value;
		current_value = current_value * normalization_ratio;		
		out << to_string(current_value) << endl;
	}
	out.clear();
	out.close();

	in.clear();
	in.seekg(0);
}


//Calculate ZCR Value for opened file
void calculate_ZCR(){	
	double previous_value, previous_value_1 = 0;
	string temp = "";
	in >> previous_value;
	cout << "\n................Calculating ZCR Value..................."<<endl;
	while (in >> temp){
		current_value = stoi(temp);
		if ((previous_value > 0 && current_value < 0) || (previous_value < 0 && current_value > 0)){
			zcr_count++;
			previous_value = current_value;
		}

	}
	cout << "ZCR value:" << zcr_count << endl;
	in.clear();
	in.seekg(0);
}

//Calculate LCR value for opened file
void calculate_LCR(){	
	string temp = "";
	while (in >> temp){
		current_value = stoi(temp);
		if (current_value >= lcr_threshold)
			lcr_count++;
	}
	cout << "\nLCR value:" << lcr_count << endl;
	in.clear();
	in.seekg(0);
}

//Calculate energy for opened file
void calculate_energy(){
	string temp = "";
	cout << "\n................Writing ENERGY values to file..................."<<endl;
	int count = 100;
	out.open(energy_file);
	while (in >> temp){
		current_value = stoi(temp);
		current_value *= 0.1;
		total_energy += (current_value*current_value);
		count--;
		if (!count){
			out << to_string(total_energy) << endl;
			energy_file_count++;
			total_energy = 0;
			count = 100;
		}
	}
	out.close();	
	in.clear();
	in.seekg(0);
}

//To Trim the main part (OLD LOGIC) Only for start Marker
void trim_waveform(){
	cout << "Trimming Waveform";
	long int total_count = 0;
	string temp = "";	
	in.close();
	in.open(normalized_file);
	while (in >> temp){
		total_count++;
		current_value = stoi(temp);	
		if (current_value >= trim_start_value){

			start_marker = total_count - 500;
			total_count = 0;
			break;
		}
	}
	in.clear();
	in.seekg(0);

	cout<<"Start Marker:"<<start_marker<< endl;

	out_t.open(trimmed_file);
	while (in >> temp){
		total_count++;
		if (total_count >= start_marker){
			out_t << temp << endl;
		}
		else{
			continue;
		}
	}
	out_t.close();
	in.close();
	in.open(input_file);
}

//Trims the waveform using window of interval shifted with 1000 values
void trim_waveform1(){
	//interval is for windows
	//writing count is for final write after markers are found
	//intital shift pointer is for shifting by 1000 values
	//shift pointer count is for reading input file
	int interval = 9000;
	long double total_max_energy = 0;
	int initial_shift_pointer = 0,shift_pointer_count=0,writing_count=0;
	long double arr_energy[50];

	string temp = "";
	cout << "\n................Trimming Waveform..................." << endl;
	int count = interval;		
	out.open("energy_trim.txt");

	//window shifting logic
	while (in >> temp){		
		if (shift_pointer_count>=initial_shift_pointer){
			
			current_value = stoi(temp);
			current_value *= 0.1;
			total_energy += (current_value*current_value);			
			count--;
			if (!count){
				
				out << to_string(total_energy) << endl;

				if (total_energy > total_max_energy){
					total_max_energy = total_energy;
					start_marker = initial_shift_pointer;
					end_marker = initial_shift_pointer + interval;
					
				}
				
				total_energy = 0;
				count = interval;
				initial_shift_pointer += 1000;
				shift_pointer_count = 0;
				in.clear();
				in.seekg(0);
			}
		}
		shift_pointer_count++;
	}
	out.close();
	in.clear();
	in.seekg(0);

	cout << "Start Marker : " << start_marker << endl;
	cout << "End Marker : " << end_marker << endl;

	//Writing to new file from start to end marker
	out_t.open(trimmed_file);
	while (in >> temp){
		writing_count++;
		if (writing_count >= start_marker && writing_count<=end_marker){
			out_t << temp << endl;
		}
		else{
			continue;
		}
	}
	out_t.close();
	in.clear();
	in.seekg(0);
}

//To Calculate Energy for certain percentage (Currently not in use)
void percentage_of_energy(){
	int temp_count = 0;
	double temp;
	double final_energy_sum = 0;
	int values_to_pick = (0.6*energy_file_count);
	in.close();
	in.open(energy_file);
	cout<<"Values to pick:"<<values_to_pick<<endl;
	while (in >> temp){
		temp_count++;
		//cout << typeid(temp).name();	
		if (temp_count == values_to_pick){
			final_energy_sum = final_energy_sum + temp;
		}
	}
	cout<<"Final Energy Sum:"<<final_energy_sum<<endl;
	in.clear();
	in.seekg(0);
	in.close();
}

//To Display the result
void result_out(){
	//percentage_of_energy();
	cout << "\n................Analyzing Result..................." << endl;
	if (zcr_count <= 1100){
		cout<<"\n\t****** IT IS A ONE ******" << endl;
	}
	else{
		cout<<"\n\t****** IT IS A SIX ******" << endl;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	//cout<<"Enter the name of input file:"<< endl;
	//cin >> input_file;			
	
	//intialize_file_names();
	
	cout << ".....Recording will be for 3 seconds......"<<endl;
	cout << "....... Recording Silence......"<<endl;
	system("Recording_Module.exe 3 silence.wav silence.txt");
	cout << "\nSilence recorded. **Press Enter to record your one/six**"<<endl;
	system("Recording_Module.exe 3 input.wav input.txt");
	cout << "\nRecording successfull **Press ENTER to proceed with program**" << endl;

	cout<<"\nReading Input from : " << input_file << endl;
	
	//Opening Recorded file for processing
	in.open(input_file);
	//Calculate DC shift and normalization ratio and create a new normalized file
	calculate_dc_shift();
	calculate_normalization_ratio();
	dc_normalize();
	in.close();

	//Trimming and making trimmed file
	in.open(normalized_file);
	trim_waveform1();
	in.close();

	//Using trimmed file for analysis
	in.open(trimmed_file);
	calculate_ZCR();
	calculate_energy();

	//Printing the calculated result
	result_out();
	in.close();
	return 0;
}

