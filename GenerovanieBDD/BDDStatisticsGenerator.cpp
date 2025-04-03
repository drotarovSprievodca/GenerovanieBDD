#include "BDDStatisticsGenerator.hpp"

BDDStatisticsGenerator::BDDStatisticsGenerator(std::string working_directory, std::string pla_files_directory, std::string csv_output_directory, bool csv_for_every_pla) {
    this->csv_output = new CSVOutput(working_directory, csv_output_directory, csv_for_every_pla);
    this->strategies = std::vector<Strategy*>();
    this->working_directory = working_directory;
    this->data_directory = this->working_directory + pla_files_directory;
}

BDDStatisticsGenerator::~BDDStatisticsGenerator() {
    for (Strategy* s : this->strategies) {
        delete s;
    }
    delete this->csv_output;
}

void BDDStatisticsGenerator::set_strategy(std::vector<Strategy*> strategies) {
    this->strategies = strategies;
}

void BDDStatisticsGenerator::add_strategy(Strategy* strategy) {
    this->strategies.push_back(strategy);
}

void BDDStatisticsGenerator::remove_strategy(int i) {
    if (i >= 0 && i < this->strategies.size()) {
        this->strategies.erase(this->strategies.begin() + i);
    }
}

void BDDStatisticsGenerator::print_strategies() {
    std::cout << "Currently set strategies:" << std::endl;
    if (this->strategies.size() == 0) {
        std::cout << "There are no strategies set !!!" << std::endl;
    } else {
        for (int i = 0; i < this->strategies.size(); ++i) {
            std::cout << std::to_string(i) << " - " << this->strategies[i]->get_strategy_name() << std::endl;
        }
    }
}

int BDDStatisticsGenerator::size_of_strategies() {
    return this->strategies.size();
}

void BDDStatisticsGenerator::get_statistics() {
    // try to open data_directory
	this->h_find = FindFirstFile((this->data_directory + "*").c_str(), &this->find_file_data);
    if (this->h_find == INVALID_HANDLE_VALUE) {
        std::cerr << "Could not open directory!" << std::endl;
        return;
    }
    std::string pla_path = "";
    std::optional<teddy::pla_file> pla_file;
    teddy::pla_file* pla = nullptr;
    
    // prepare header
    std::string header = "File;Function;Number of variables";
    for (Strategy* s : this->strategies) {
        header += ";" + s->to_string();
    }
    header += "\n";
    this->csv_output->set_header(header);
    
    // iterate through all pla files in data_directory for each strategy
    do {
        // skip . and .. in data_directory
        if (std::string(this->find_file_data.cFileName) == "." || std::string(this->find_file_data.cFileName) == "..") {
            continue;
        }

        // get pla file from directory and load its content
        pla_path = this->data_directory + this->find_file_data.cFileName;
        pla_file = teddy::pla_file::load_file(pla_path);
        if (!pla_file.has_value()) { 
            continue; 
        }
        pla = &pla_file.value();

        // number of variables in this pla file
        int number_of_vars = pla->get_variable_count();

        // number of functions in this pla file
        int number_of_functions = pla->get_function_count();

        // get file name without extension
        std::filesystem::path file_path(this->find_file_data.cFileName);
        std::string file_name_without_extension = "file_" + file_path.stem().string();

        // Creating manager with default order of variables
        teddy::bss_manager default_manager(number_of_vars, 100'000);

        // Disable automatic variable reordering.
        default_manager.set_auto_reorder(false);

        // prepare csv file for writing to
        this->csv_output->open_csv(file_name_without_extension);

        std::cout << "Processing " << std::to_string(number_of_functions) << " function(s) (each with " << std::to_string(number_of_vars) << " variables) from file: " + std::string(find_file_data.cFileName) + " ..." << std::endl;
        
        for (int i = 0; i < number_of_functions; ++i) {
            this->csv_output->write_info_about_function(this->find_file_data.cFileName, i + 1, number_of_vars);
            for (Strategy* strategy : this->strategies) {
                strategy->process_function(default_manager, number_of_vars, pla, this->csv_output, i, file_name_without_extension);
            }
            this->csv_output->new_line();
        }

        // close csv file if there is one csv per pla file
        if (this->csv_output->get_csv_for_every_pla()) {
            this->csv_output->close_csv();
        }

    } while (FindNextFile(this->h_find, &this->find_file_data) != 0);
    
    // close csv file if there is only one csv for all pla files
    if (!this->csv_output->get_csv_for_every_pla()) {
        this->csv_output->close_csv();
    }

    FindClose(this->h_find);
}
