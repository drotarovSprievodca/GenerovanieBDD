#include "CSVOutput.hpp"

CSVOutput::CSVOutput(std::string working_directory, std::string csv_output_directory, bool csv_for_every_pla) {
	this->csv_for_every_pla = csv_for_every_pla;
	this->working_directory = working_directory;
	this->csv_directory = this->working_directory + csv_output_directory;
	
	for (const auto& entry : std::filesystem::directory_iterator(this->csv_directory)) {
		std::filesystem::remove_all(entry.path());
	}
}

bool CSVOutput::set_for_write(std::string pla_file_name) {
	if (this->csv_for_every_pla) {
		std::string file_name = "from_" + pla_file_name + ".csv";
		if (this->file.is_open()) {
			std::cerr << "Something went wrong, file: " << file_name << " is already opened and it should not be !!!" << std::endl;
			return false;
		}
		file.open(this->csv_directory + file_name);
		if (!this->file.is_open()) {
			std::cerr << "Failed to open the file: " << file_name << std::endl;
			return false;
		}
		return true;
	} else {
		std::string file_name = "from_all.csv";
		if (!this->file.is_open()) {
			if (!this->first_pla_file_found) {
				this->first_pla_file_found = true;
				file.open(this->csv_directory + file_name);
				if (!this->file.is_open()) {
					std::cerr << "Failed to open the file: " << file_name << std::endl;
					return false;
				}
				return true;
			}
			std::cerr << "Something went wrong, file: " << file_name << " is not opened and it should be !!!" << std::endl;
			return false;
		}
		return true;
	}
}

bool CSVOutput::unset_for_write() {
	
}
