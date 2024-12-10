#pragma once

#include <libteddy/core.hpp>
#include <libteddy/reliability.hpp>
#include <windows.h>
#include <filesystem>

#include "CSVOutput.hpp"
#include "OriginalOrder.hpp"
#include "RandomOrder.hpp"
#include "DerivativesBasedOrder.hpp"

class BDDStatisticsGenerator {
	public:
		BDDStatisticsGenerator(std::string working_directory, std::string pla_files_directory, std::string csv_output_directory, bool csv_for_every_pla);
		~BDDStatisticsGenerator();
		void set_strategy(std::vector<Strategy*> strategies);
		void add_strategy(Strategy* strategy);
		void remove_strategy(int i);
		void print_strategies();
		void get_statistics();
	private:
		std::string working_directory;
		std::string data_directory;  // absolute path to directory with all pla files
		WIN32_FIND_DATA find_file_data;  // structure that contains information about data_directory
		HANDLE h_find;  // pointer used for search process of directory
		CSVOutput* csv_output;
		std::vector<Strategy*> strategies;
};