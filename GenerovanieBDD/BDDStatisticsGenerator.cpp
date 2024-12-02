#include "BDDStatisticsGenerator.hpp"

BDDStatisticsGenerator::BDDStatisticsGenerator(std::string working_directory, std::string pla_files_directory) {
	this->strategies = std::vector<Strategy*>();
    this->working_directory = working_directory;
    this->data_directory = this->working_directory + pla_files_directory;
}

BDDStatisticsGenerator::~BDDStatisticsGenerator() {
    for (Strategy* s : this->strategies) {
        delete s;
    }
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
    for (Strategy* s : this->strategies) {
        std::cout << s->to_string() << std::endl;
    }
}

void BDDStatisticsGenerator::get_statistics() {
    // iterate through all strategies
	for (Strategy* strategy : strategies) {

        double sum_of_node_counts = 0.0; // sum of all nodes of all functions of all pla files in data_directory
        double number_of_diagrams = 0.0; // number of all diagrams of all pla files (diagram == function)

        // try to open data_directory
		this->h_find = FindFirstFile((this->data_directory + "*").c_str(), &this->find_file_data);
        if (this->h_find == INVALID_HANDLE_VALUE) {
            std::cerr << "Could not open directory!" << std::endl;
            return;
        }
        std::string pla_path = "";
        std::optional<teddy::pla_file> pla_file;
        teddy::pla_file* pla = nullptr;

        // iterate through all pla files in data_directory for each strategy
        do {
            // skip . and .. in data_directory
            if (std::string(this->find_file_data.cFileName) == "." || std::string(this->find_file_data.cFileName) == "..") {
                continue;
            }

            // get pla file from directory and load his content
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

            std::cout << "There is " << std::to_string(number_of_functions) << " functions with " << std::to_string(number_of_vars) << " variables in file: " + std::string(find_file_data.cFileName) << std::endl;

            // iterate through all functions in pla file
            for (int i = 0; i < number_of_functions; ++i) {
                sum_of_node_counts += strategy->process_function(default_manager, number_of_vars, pla, i);
                number_of_diagrams++;
            }

        } while (FindNextFile(this->h_find, &this->find_file_data) != 0);

        FindClose(this->h_find);

        double average_number_of_nodes = sum_of_node_counts / number_of_diagrams;
        std::cout << "Average number of nodes in strategy: " + strategy->to_string() + " is: " + std::to_string(average_number_of_nodes) << std::endl;
	}	
}
