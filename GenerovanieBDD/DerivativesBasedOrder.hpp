#pragma once

#include <filesystem>
#include <algorithm>

#include "Strategy.hpp"

struct var {
    int variable;
    double true_density;
};

class DerivativesBasedOrder : public Strategy {
    private:
        bool ascending;
        void get_td_of_all_vars_in_function(teddy::bss_manager& default_manager, std::vector<var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram, int which_diagram);
    public:
        DerivativesBasedOrder(bool use_var_reordering_heuristics, bool ascending);
        ~DerivativesBasedOrder();
        double process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, int which_function) override;
        std::string to_string() override;
};