#pragma once

#include <filesystem>
#include <algorithm>

#include "Strategy.hpp"

struct td_var {
    int variable;
    double true_density;
};

class DerivativesBasedOrder : public Strategy {
    private:
        bool ascending;
    public:
        DerivativesBasedOrder(bool use_var_reordering_heuristics, bool ascending);
        ~DerivativesBasedOrder();
        void get_td_of_all_vars_in_function(teddy::bss_manager& default_manager, std::vector<td_var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram, int which_diagram);
        void process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) override;
        std::string to_string() override;
};