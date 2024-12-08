#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

class CSVOutput {
	private:
		std::string csv_directory;
		std::string working_directory;
		bool csv_for_every_pla = true;
		bool first_pla_file_found = false;
		int last_coulmn_of_previous_strategy;
		std::ofstream file;
	public: 
		CSVOutput(std::string working_directory, std::string csv_output_directory, bool csv_for_every_pla);
		bool set_for_write(std::string pla_file_name);
		bool unset_for_write();
		void write_info_about_function();
		void write_new_stats();
};