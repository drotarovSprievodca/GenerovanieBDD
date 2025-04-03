#pragma once

#include <random>

#include "Strategy.hpp"

class RandomOrder : public Strategy {
    private:
        std::mt19937 eng;
        int number_of_replications;
        double sum_of_node_counts_in_function_for_all_rep;
    public:
        RandomOrder(bool use_var_reordering_heuristics, int number_of_replications, bool generate_graph_before_order, bool generate_graph_after_order);
        ~RandomOrder();
        void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) override;
        std::string to_string() override;
        std::string get_strategy_name() override;
};