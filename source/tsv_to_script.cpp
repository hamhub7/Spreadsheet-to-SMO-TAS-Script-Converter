/* Author: Colin Hegarty
 * Date: 5/24/19
 * This program is used to parse the input spreadsheets and output a script
 * to use for SMO TAS.
 */
 
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string>	
#include <cstring>
#include <map>
#include <direct.h>
#include "parse.h"

std::vector<std::string> choose_files (std::string base_tsv, bool tsv_folder);
void write_scripts (const std::vector<std::string>& file_names, std::string delete_param, bool script_folder);
void convert_to_script (std::string table_file_name, std::string out_file_name);
std::string increment_string (std::string num);
void print_line (std::ofstream& file, const std::vector<std::string>& line);
void delete_tsv (const std::vector<std::string>& file_names, std::string delete_param);
int delete_file (std::string file_name);

int main (int argc, char* argv[]) {
	std::string modify_file = "settings.txt";
	Parameters params(modify_file);
	std::string base_tsv = params.base_tsv();
	write_scripts(choose_files(base_tsv, params.tsv_folder()), params.delete_tsv(), params.script_folder());
}

// Make a vector of files names for the .tsv's that will be converted.
// The first one is the name in the "modify" file and will be converted into
// script0.txt. Each .tsv after that is assumed to be the same as the first
// but have "(<i>)" at the end, where <i> starts with 1 and increments. Each one
// will be converted to script<i>.txt
std::vector<std::string> choose_files (std::string base_tsv, bool tsv_folder) {
	std::vector<std::string> file_names;
	std::string file_name;
	
	// Checks up to 10 files (only 10 scripts are supported at a time)
	for (unsigned int i = 0; i < 10; ++i) {
		file_name = "";
		// If the .tsv's are in a separate folder, include that in the file name
		if (tsv_folder) {
			file_name += "tsv\\";
		}
		// Creates the file name for the current .tsv
		if (i == 0) {
			file_name += base_tsv + ".tsv";
		} else {
			file_name += base_tsv + "(" + std::to_string(i) + ").tsv";
		}
		// Checks that the file exists before adding it to the list
		std::ifstream file(file_name.c_str());
		if (file.good()) {
			file_names.push_back(file_name);
		}
		file.close();
	}
	return file_names;
}

// Loops through the .tsv's and converts each one to a script.
void write_scripts (const std::vector<std::string>& file_names, std::string delete_param, bool script_folder) {
	std::string script_name;
	
	for (unsigned int i = 0; i < file_names.size(); ++i) {
		script_name = "";
		// If the scripts are supposed to be in a separate folder, include that in the file name
		if (script_folder) {
			script_name += "scripts\\";
			_mkdir(script_name.c_str());
		}
		script_name += "script" + std::to_string(i) + ".txt";
		convert_to_script(file_names[i], script_name);
	}
	delete_tsv(file_names, delete_param);
}

// Executes the conversion process 
void convert_to_script (std::string table_file_name, std::string out_file_name) {
	// Each vector is a row, each string is a cell in that row.
	std::vector<std::vector<std::string> > table = parse_sheet("\t",table_file_name);
	std::ofstream file(out_file_name.c_str());
	bool bad;
	// Read the entries of table, and print the desired info for relevant flights
	for (unsigned int i = 0; i < table.size(); i++) {
		if (!good_tsv_line(table[i])) continue;
		print_line(file, table[i]);
	}
	file.close();
}

// Print the line to the file with correct formatting for the script.
void print_line (std::ofstream& file, const std::vector<std::string>& line) {
	// Frame number
	file << line[0] << " ";
	// Button presses
	file << line[1] << " ";
	// Left stick then right stick
	file << line[2] << ";" << line[3] << " ";
	file << line[4] << ";" << line[5];
	// The code below would include comments after the last input
	#ifdef COMMENTS
	for (unsigned int i = 6; i < line.size(); ++i) {
		file << "\t" << line[i];
	}
	#endif
	file << std::endl;
}

// Check from the "modify" file if the .tsv's should be deleted, and if so, do it
void delete_tsv (const std::vector<std::string>& file_names, std::string delete_param) {
	// Doesn't delete original file if "none" or "all but original" is chosen
	if (delete_param == "all") {
		delete_file(file_names[0]);
	}
	// Deletes all duplicate files if "all" or "all but original" is chosen
	if (delete_param != "none") {
		for (unsigned int i = 1; i < file_names.size(); ++i) {
			delete_file(file_names[i]);
		}
	}
}

int delete_file (std::string file_name) {
	int result = remove(file_name.c_str());
	
	if (result != 0) {
		perror("Error deleting file");
		std::cerr << "ERROR: failed to delete " << file_name << std::endl;
	}
	return result;
}