#include "CSVOutput.hpp"

CSVOutput::CSVOutput(std::string working_directory, std::string csv_output_directory, bool csv_for_every_pla) {
	this->set_csv_for_every_pla(csv_for_every_pla);
	this->working_directory = working_directory;
	this->csv_directory = this->working_directory + csv_output_directory;
	
	for (const auto& entry : std::filesystem::directory_iterator(this->csv_directory)) {
		std::filesystem::remove_all(entry.path());
	}
}

CSVOutput::~CSVOutput() {
	if (this->csv_file_stream.is_open()) {
		this->csv_file_stream.close();
	}
}

void CSVOutput::set_csv_for_every_pla(bool csv_for_every_pla) {
	this->csv_for_every_pla = csv_for_every_pla;
	if (!this->csv_for_every_pla) {
		this->csv_file_name = "from_all.csv";
	}
}

bool CSVOutput::get_csv_for_every_pla() {
	return this->csv_for_every_pla;
}

bool CSVOutput::open_csv(std::string pla_file_name) {
	if (this->csv_for_every_pla) {
		this->csv_file_name = "from_" + pla_file_name + ".csv";
		if (this->csv_file_stream.is_open()) {
			std::cerr << "Something went wrong, file: " << this->csv_file_name << " is already opened and it should not be !!!" << std::endl;
			return false;
		}
		csv_file_stream.open(this->csv_directory + this->csv_file_name);
		csv_file_stream << this->header;
		if (!this->csv_file_stream.is_open()) {
			std::cerr << "Failed to open the file: " << this->csv_file_name << std::endl;
			return false;
		}
		return true;

	} else {
		if (!this->csv_file_stream.is_open()) {
			if (!this->first_pla_file_found) {
				this->first_pla_file_found = true;
				csv_file_stream.open(this->csv_directory + this->csv_file_name);
				csv_file_stream << this->header;
				if (!this->csv_file_stream.is_open()) {
					std::cerr << "Failed to open the file: " << this->csv_file_name << std::endl;
					return false;
				}
				return true;
			}
			std::cerr << "Something went wrong, file: " << this->csv_file_name << " is not opened and it should be !!!" << std::endl;
			return false;
		}
		return true;
	}
}

void CSVOutput::set_header(std::string header) {
	this->header = header;
}

bool CSVOutput::close_csv() {
	if (!this->csv_file_stream.is_open()) {
		std::cerr << "Something went wrong, file: " << csv_file_name << " is not opened and it should be !!!" << std::endl;
		return false;
	}
	this->csv_file_stream.close();
	if (this->csv_file_stream.is_open()) {
		std::cerr << "Failed to close the file: " << this->csv_file_name << std::endl;
		return false;
	}
	return true;
}

void CSVOutput::write_info_about_function(std::string file_name, int which_function, int number_of_vars) {
	this->csv_file_stream << file_name << ";" << "f" << std::to_string(which_function) << ";" << std::to_string(number_of_vars);
}

void CSVOutput::write_strategy_times(std::vector<double>& strategy_timers) {
	std::string timers = "time(sec);;";
	for (double timer : strategy_timers) {
		timers += ";";
		timers += this->round_to_number_of_dec_places(timer, 2);
		timers += ";";
	}
	this->csv_file_stream << timers;
}

std::string CSVOutput::round_to_number_of_dec_places(double number_to_round, int number_of_dec_places) {
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(number_of_dec_places) << number_to_round;
	std::string rounded_number = oss.str();
	for (char& c : rounded_number) {
		if (c == '.') {
			c = ',';
		}
	}
	return rounded_number;
}

void CSVOutput::write_new_stats(std::string order, double node_count) {
	this->csv_file_stream << ";" << order;
	this->csv_file_stream << ";" << this->round_to_number_of_dec_places(node_count, 2);
}

void CSVOutput::new_line() {
	this->csv_file_stream << "\n";
}
