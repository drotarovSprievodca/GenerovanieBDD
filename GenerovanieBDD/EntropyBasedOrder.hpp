#pragma once

#include "Strategy.hpp"
#include <cmath>

struct ce_var {
    int variable;
    double conditional_entropy;
};

class EntropyBasedOrder : public Strategy {

public:
    EntropyBasedOrder(bool use_var_reordering_heuristics);
    ~EntropyBasedOrder();
    void get_ce_of_all_vars_in_function(teddy::bss_manager& default_manager, std::vector<ce_var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram, int which_diagram);
    void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) override;
    std::string to_string() override;
};