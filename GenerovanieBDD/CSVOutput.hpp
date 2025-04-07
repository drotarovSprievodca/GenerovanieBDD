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
		std::ofstream csv_file_stream;
		std::string csv_file_name;
		std::string header;
	public: 
		CSVOutput(std::string working_directory, std::string csv_output_directory, bool csv_for_every_pla);
		~CSVOutput();
		void set_csv_for_every_pla(bool csv_for_every_pla);
		bool get_csv_for_every_pla();
		bool open_csv(std::string pla_file_name);
		void set_header(std::string header);
		bool close_csv();
		void write_info_about_function(std::string file_name, int which_function, int number_of_vars);
		void write_strategy_times(std::vector<double>& strategy_timers);
		std::string round_to_number_of_dec_places(double number_to_round, int number_of_dec_places);
		void write_new_stats(std::string order, double node_count);
		void new_line();
};